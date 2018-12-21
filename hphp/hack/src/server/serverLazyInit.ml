(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

(* Lazy Initialization:

   During Lazy initialization, hh_server tries to do as little work as possible.
   If we load from saved state, our steps are:

     Load from saved state -> Parse dirty files -> Naming -> Dirty Typecheck

   Otherwise, we fall back to something similar to eager init, but with lazy
   decl and parse turned on:

     Full Parsing -> Naming -> Full Typecheck (with lazy decl)
*)

open Core_kernel
open Result.Export
open Reordered_argument_collections
open SearchServiceRunner
open ServerCheckUtils
open ServerEnv
open ServerInitCommon
open ServerInitTypes
open String_utils

module DepSet = Typing_deps.DepSet
module Dep = Typing_deps.Dep
module SLC = ServerLocalConfig

let lock_and_load_deptable (fn: string) ~(ignore_hh_version: bool) : unit =
  (* The sql deptable must be loaded in the master process *)
  try
    (* Take a lock on the info file for the sql *)
    LoadScriptUtils.lock_saved_state fn;
    let read_deptable_time =
      SharedMem.load_dep_table_sqlite fn ignore_hh_version
    in
    Hh_logger.log
      "Reading the dependency file took (sec): %d" read_deptable_time;
    HackEventLogger.load_deptable_end read_deptable_time;
  with
  | SharedMem.Sql_assertion_failure 11
  | SharedMem.Sql_assertion_failure 14 as e -> (* SQL_corrupt *)
    LoadScriptUtils.delete_corrupted_saved_state fn;
    raise e

(* download_and_load_state_exn does these things:
 * mk_state_future which synchronously downloads ss and kicks of async dirty query
 * lock_and_load_deptable
 * load_saved_state
 * synchronously wait 200s for the async dirty query to finish
*)
let download_and_load_state_exn
    ~(use_canary: bool)
    ~(target: ServerMonitorUtils.target_saved_state option)
    ~(genv: ServerEnv.genv)
    ~(root: Path.t)
  : (loaded_info, error) result =
  let open ServerMonitorUtils in
  let saved_state_handle = match target with
    | None -> None
    | Some { saved_state_everstore_handle; target_svn_rev; watchman_mergebase } ->
      Some {
        State_loader.saved_state_everstore_handle = saved_state_everstore_handle;
        saved_state_for_rev = (Hg.Svn_rev target_svn_rev);
        watchman_mergebase;
      } in
  let ignore_hh_version = ServerArgs.ignore_hh_version genv.options in
  let use_prechecked_files = ServerPrecheckedFiles.should_use genv.options genv.local_config in

  let state_future : (State_loader.native_load_result, State_loader.error) result =
    State_loader.mk_state_future
      ~config:genv.local_config.SLC.state_loader_timeouts
      ~use_canary ?saved_state_handle
      ~config_hash:(ServerConfig.config_hash genv.config) root
      ~ignore_hh_version
      ~use_prechecked_files in

  match state_future with
  | Error error ->
    Error (Lazy_init_loader_failure error)
  | Ok result ->
    lock_and_load_deptable result.State_loader.deptable_fn ~ignore_hh_version;
    let load_decls = genv.local_config.SLC.load_decls_from_saved_state in
    let (old_saved, old_errors) =
      SaveStateService.load_saved_state result.State_loader.saved_state_fn
        ~load_decls in

    let t = Unix.time () in
    match result.State_loader.dirty_files |> Future.get ~timeout:200 with
    | Error error ->
      Error (Lazy_init_dirty_files_failure error)
    | Ok (dirty_master_files, dirty_local_files) -> begin
        let () = HackEventLogger.state_loader_dirty_files t in
        let list_to_set x =
          List.map x Relative_path.from_root |> Relative_path.set_of_list in
        let dirty_master_files = list_to_set dirty_master_files in
        let dirty_local_files = list_to_set dirty_local_files in
        Ok {
          saved_state_fn = result.State_loader.saved_state_fn;
          corresponding_rev = result.State_loader.corresponding_rev;
          mergebase_rev = result.State_loader.mergebase_rev;
          dirty_master_files;
          dirty_local_files;
          old_saved;
          old_errors;
          state_distance = Some result.State_loader.state_distance;
        }
      end

let use_precomputed_state_exn
    (genv: ServerEnv.genv)
    (info: ServerArgs.saved_state_target_info)
  : loaded_info =
  let ignore_hh_version = ServerArgs.ignore_hh_version genv.options in
  let { ServerArgs.
        saved_state_fn;
        corresponding_base_revision;
        deptable_fn;
        changes;
        prechecked_changes;
      } = info in
  lock_and_load_deptable deptable_fn ~ignore_hh_version;
  let changes = Relative_path.set_of_list changes in
  let prechecked_changes = Relative_path.set_of_list prechecked_changes in
  let load_decls = genv.local_config.SLC.load_decls_from_saved_state in
  let (old_saved, old_errors) =
    SaveStateService.load_saved_state saved_state_fn ~load_decls
  in
  {
    saved_state_fn;
    corresponding_rev = (Hg.Svn_rev (int_of_string (corresponding_base_revision)));
    mergebase_rev  = None;
    dirty_master_files = prechecked_changes;
    dirty_local_files = changes;
    old_saved;
    old_errors;
    state_distance = None;
  }

(* Run naming from a fast generated from saved state.
 * No errors are generated because we assume the fast is directly from
 * a clean state.
*)
let naming_with_fast (fast: FileInfo.names Relative_path.Map.t) (t: float) : float =
  Relative_path.Map.iter fast ~f:begin fun k info ->
    let { FileInfo.n_classes=classes;
          n_types=typedefs;
          n_funs=funs;
          n_consts=consts} = info in
    NamingGlobal.ndecl_file_fast k ~funs ~classes ~typedefs ~consts
  end;
  HackEventLogger.fast_naming_end t;
  let hs = SharedMem.heap_size () in
  Hh_logger.log "Heap size: %d" hs;
  (Hh_logger.log_duration "Naming fast" t)

(*
 * In eager initialization, this is done at the parsing step with
 * parsing hooks. During lazy init, need to do it manually from the fast
 * instead since we aren't parsing the codebase.
 *)
let update_search (genv: ServerEnv.genv) (saved: FileInfo.saved_state_info) (t: float) : float =
  (* Don't update search index when in check mode *)
  (* We can't use is_check_mode here because we want to
     skip this step even while saving saved states.
  *)
  if ServerArgs.check_mode genv.options then t else
    (* Only look at Hack files *)
    let fast = FileInfo.saved_to_hack_files saved in
    (* Filter out non php files *)
    let fast = Relative_path.Map.filter fast
        ~f:(fun s _ -> FindUtils.path_filter s) in

    Relative_path.Map.iter fast
      ~f: (fun fn names ->
          SearchServiceRunner.update (fn, (SearchServiceRunner.Fast names));
        );
    HackEventLogger.update_search_end t;
    Hh_logger.log_duration "Loading search indices" t

(* Prechecked files are gated with a flag and not supported in AI/check/saving
 * of saved state modes. *)
let use_prechecked_files (genv: ServerEnv.genv) : bool =
  ServerPrecheckedFiles.should_use genv.options genv.local_config &&
  ServerArgs.ai_mode genv.options = None &&
  (not @@ is_check_mode genv.options) &&
  ServerArgs.save_filename genv.options = None

let get_dirty_fast
    (old_fast: FileInfo.names Relative_path.Map.t)
    (fast: FileInfo.names Relative_path.Map.t)
    (dirty: Relative_path.Set.t)
  : FileInfo.names Relative_path.Map.t =
  Relative_path.Set.fold dirty ~f:begin fun fn acc ->
    let dirty_fast = Relative_path.Map.get fast fn in
    let dirty_old_fast = Relative_path.Map.get old_fast fn in
    let fast = Option.merge dirty_old_fast dirty_fast FileInfo.merge_names in
    match fast with
    | Some fast -> Relative_path.Map.add acc ~key:fn ~data:fast
    | None -> acc
  end ~init:Relative_path.Map.empty

let names_to_deps (names: FileInfo.names) : DepSet.t =
  let {FileInfo.n_funs; n_classes; n_types; n_consts} = names in
  let add_deps_of_sset dep_ctor sset depset =
    SSet.fold sset ~init:depset ~f:begin fun n acc ->
      DepSet.add acc (Dep.make (dep_ctor n))
    end
  in
  let deps = add_deps_of_sset (fun n -> Dep.Fun n) n_funs DepSet.empty in
  let deps = add_deps_of_sset (fun n -> Dep.FunName n) n_funs deps in
  let deps = add_deps_of_sset (fun n -> Dep.Class n) n_classes deps in
  let deps = add_deps_of_sset (fun n -> Dep.Class n) n_types deps in
  let deps = add_deps_of_sset (fun n -> Dep.GConst n) n_consts deps in
  let deps = add_deps_of_sset (fun n -> Dep.GConstName n) n_consts deps in
  deps

(** Compare declarations loaded from the saved state to declarations based on
    the current versions of dirty files. This lets us check a smaller set of
    files than the set we'd check if old declarations were not available.
    To be used only when load_decls_from_saved_state is enabled. *)
let get_files_to_recheck
    (genv: ServerEnv.genv)
    (env: ServerEnv.env)
    (old_fast: FileInfo.names Relative_path.Map.t)
    (new_fast: FileInfo.names Relative_path.Map.t)
    (dirty_fast: FileInfo.names Relative_path.Map.t)
    (files_to_redeclare: Relative_path.Set.t)
  : Relative_path.Set.t =
  let bucket_size = genv.local_config.SLC.type_decl_bucket_size in
  let fast =
    Relative_path.Set.fold files_to_redeclare
      ~init:Relative_path.Map.empty
      ~f:begin fun path acc ->
        match Relative_path.Map.get dirty_fast path with
        | Some info -> Relative_path.Map.add acc path info
        | None -> acc
      end
  in
  let get_classes path =
    let old_names = Relative_path.Map.get old_fast path in
    let new_names = Relative_path.Map.get new_fast path in
    let classes_from_names x = x.FileInfo.n_classes in
    let old_classes = Option.map old_names classes_from_names in
    let new_classes = Option.map new_names classes_from_names in
    Option.merge old_classes new_classes SSet.union
    |> Option.value ~default:SSet.empty
  in
  let dirty_names =
    Relative_path.Map.fold dirty_fast
      ~init:FileInfo.empty_names ~f:(fun _ -> FileInfo.merge_names)
  in
  Decl_redecl_service.oldify_type_decl ~bucket_size
    genv.workers get_classes FileInfo.empty_names dirty_names;
  let _, _, to_redecl, to_recheck =
    Decl_redecl_service.redo_type_decl ~conservative_redecl:false
      ~bucket_size genv.workers env.tcopt dirty_names fast in
  Decl_redecl_service.remove_old_defs ~bucket_size genv.workers dirty_names;
  let deps = Typing_deps.add_all_deps to_redecl in
  let deps = Typing_deps.DepSet.union deps to_recheck in
  Typing_deps.get_files deps

(* We start of with a list of files that have changed since the state was
 * saved (dirty_files), and two maps of the class / function declarations
 * -- one made when the state was saved (old_fast) and one made for the
 * current files in the repository (new_fast). We grab the declarations from
 * both, to account for both the declaratons that were deleted and those that
 * are newly created. Then we use the deptable to figure out the files that
 * referred to them. Finally we recheck the lot.
 *
 * Args:
 *
 * genv, env : environments
 * old_fast: old file-ast from saved state
 * new_fast: newly parsed file ast
 * dirty_files: we need to typecheck these and,
 *    since their decl have changed, also all of their dependencies
 * similar_files: we only need to typecheck these,
 *    not their dependencies since their decl are unchanged
 **)
let type_check_dirty
    (genv: ServerEnv.genv)
    (env: ServerEnv.env)
    (old_fast: FileInfo.names Relative_path.Map.t)
    (new_fast: FileInfo.names Relative_path.Map.t)
    (dirty_master_files: Relative_path.Set.t)
    (dirty_local_files: Relative_path.Set.t)
    (similar_files: Relative_path.Set.t)
    (t: float)
  : ServerEnv.env * float =
  let dirty_files =
    Relative_path.Set.union dirty_master_files dirty_local_files in
  let start_t = Unix.gettimeofday () in
  let dirty_fast = get_dirty_fast old_fast new_fast dirty_files in
  let names s = Relative_path.Map.fold dirty_fast ~f:begin fun k v acc ->
      if Relative_path.Set.mem s k then FileInfo.merge_names v acc
      else acc
    end ~init:FileInfo.empty_names in
  let master_deps = names dirty_master_files |> names_to_deps in
  let local_deps = names dirty_local_files |> names_to_deps in
  (* Include similar_files in the dirty_fast used to determine which loaded
     declarations to oldify. This is necessary because the positions of
     declarations may have changed, which affects error messages and FIXMEs. *)
  let get_files_to_recheck =
    get_files_to_recheck genv env old_fast new_fast @@
    extend_fast dirty_fast env.files_info similar_files in

  let env, to_recheck = if use_prechecked_files genv then begin
      (* Start with dirty files and fan-out of local changes only *)
      let to_recheck =
        if genv.local_config.SLC.load_decls_from_saved_state
        then get_files_to_recheck dirty_local_files
        else
          let deps = Typing_deps.add_all_deps local_deps in
          Typing_deps.get_files deps
      in
      ServerPrecheckedFiles.set env (Initial_typechecking {
          rechecked_files = Relative_path.Set.empty;
          dirty_local_deps = local_deps;
          dirty_master_deps = master_deps;
          clean_local_deps = Typing_deps.DepSet.empty;
        }), to_recheck
    end else begin
      (* Start with full fan-out immediately *)
      let to_recheck =
        if genv.local_config.SLC.load_decls_from_saved_state
        then get_files_to_recheck dirty_files
        else
          let deps = Typing_deps.DepSet.union master_deps local_deps in
          let deps = Typing_deps.add_all_deps deps in
          Typing_deps.get_files deps
      in
      env, to_recheck
    end in
  (* We still need to typecheck files whose declarations did not change *)
  let to_recheck = Relative_path.Set.union to_recheck similar_files in
  let fast = extend_fast dirty_fast env.files_info to_recheck in
  let result = type_check genv env fast t in
  HackEventLogger.type_check_dirty ~start_t
    ~dirty_count:(Relative_path.Set.cardinal dirty_files)
    ~recheck_count:(Relative_path.Set.cardinal to_recheck);
  Hh_logger.log "ServerInit type_check_dirty count: %d. recheck count: %d"
    (Relative_path.Set.cardinal dirty_files)
    (Relative_path.Set.cardinal to_recheck);
  result

(* get the (untracked, tracked) build targets *)
let get_build_targets (env: ServerEnv.env) : Relative_path.Set.t * Relative_path.Set.t =
  let untracked, tracked = BuildMain.get_live_targets env in
  let untracked =
    List.map untracked Relative_path.from_root in
  let tracked =
    List.map tracked Relative_path.from_root in
  Relative_path.set_of_list untracked, Relative_path.set_of_list tracked

let get_updates_exn
    ~(genv: ServerEnv.genv)
    ~(root: Path.t)
  : Relative_path.Set.t =
  let open ServerNotifierTypes in
  genv.wait_until_ready ();
  match genv.notifier_async () with
  | Notifier_state_enter _
  | Notifier_state_leave _
  | Notifier_unavailable ->
    Relative_path.Set.empty
  | Notifier_synchronous_changes updates
  | Notifier_async_changes updates ->
    let root = Path.to_string root in
    let filter p = string_starts_with p root && FindUtils.file_filter p in
    SSet.filter updates ~f:filter
    |> Relative_path.relativize_set Relative_path.Root

let tls_bug_re = Str.regexp_string "fburl.com/tls_debug"

let matches_re re s =
  let pos = try Str.search_forward re s 0 with Caml.Not_found -> -1 in
  pos > -1

(* If we fail to load a saved state, fall back to typechecking everything *)
let fallback_init
    (genv: ServerEnv.genv)
    (env: ServerEnv.env)
    (err: error)
  : ServerEnv.env * float =
  SharedMem.cleanup_sqlite ();
  if err <> Lazy_init_no_load_approach then begin
    let err_str = error_to_verbose_string err in
    HackEventLogger.load_state_exn err_str;
    Hh_logger.log "Could not load saved state: %s" err_str;
    let warning = if matches_re tls_bug_re err_str
      then ClientMessages.tls_bug_msg
      else ClientMessages.load_state_not_found_msg
    in
    ServerProgress.send_to_monitor (MonitorRpc.PROGRESS_WARNING (Some warning));
  end;
  let get_next, t = indexing genv in
  (* The full_fidelity_parser currently works better in both memory and time
     with a full parse rather than parsing decl asts and then parsing full ones *)
  let lazy_parse = not genv.local_config.SLC.use_full_fidelity_parser in
  (* full init - too many files to trace all of them *)
  let trace = false in
  let env, t = parsing ~lazy_parse genv env ~get_next t ~trace in
  if not (ServerArgs.check_mode genv.options) then
    SearchServiceRunner.update_fileinfo_map env.files_info;
  let t = update_files genv env.files_info t in
  let env, t = naming env t in
  let fast = FileInfo.simplify_fast env.files_info in
  let failed_parsing = Errors.get_failed_files env.errorl Errors.Parsing  in
  let fast = Relative_path.Set.fold failed_parsing
      ~f:(fun x m -> Relative_path.Map.remove m x) ~init:fast in
  type_check genv env fast t

let post_saved_state_initialization
    ~(genv: ServerEnv.genv)
    ~(env: ServerEnv.env)
    ~(loaded_info: loaded_info)
    ~(changed_while_parsing: Relative_path.Set.t)
  : (ServerEnv.env * float) =
  let trace = genv.local_config.SLC.trace_parsing in
  let hg_aware = genv.local_config.SLC.hg_aware in
  let {
    dirty_local_files;
    dirty_master_files;
    old_saved;
    mergebase_rev;
    old_errors;
    _} = loaded_info in

  if hg_aware then Option.iter mergebase_rev ~f:ServerRevisionTracker.initialize;
  Bad_files.check dirty_local_files;
  Bad_files.check changed_while_parsing;

  let (decl_and_typing_error_files, naming_and_parsing_error_files) =
    SaveStateService.partition_error_files_tf
      old_errors
      [ Errors.Decl; Errors.Typing; ] in

  let (_old_parsing_phase, old_parsing_error_files) =
    match List.find
            old_errors
            ~f:(fun (phase, _files) -> (phase = Errors.Parsing)) with
    | Some (a, b) -> (a, b)
    | None -> (Errors.Parsing, Relative_path.Set.empty)
  in

  Hh_logger.log
    "Number of files with Decl and Typing errors: %d"
    (Relative_path.Set.cardinal decl_and_typing_error_files);

  Hh_logger.log
    "Number of files with Naming and Parsing errors: %d"
    (Relative_path.Set.cardinal naming_and_parsing_error_files);

  let (decl_and_typing_error_files, naming_and_parsing_error_files) =
    SaveStateService.partition_error_files_tf
      old_errors
      [ Errors.Decl; Errors.Typing; ] in

  (* Parse and name all dirty files uniformly *)
  let dirty_files =
    Relative_path.Set.union naming_and_parsing_error_files (
      Relative_path.Set.union dirty_master_files dirty_local_files) in
  let build_targets, tracked_targets = get_build_targets env in
  let t = Unix.gettimeofday () in
  (* Build targets are untracked by version control, so we must always
   * recheck them. While we could query hg / git for the untracked files,
   * it's much slower. *)
  let dirty_files =
    Relative_path.Set.union dirty_files build_targets in
  let dirty_files =
    Relative_path.Set.union dirty_files changed_while_parsing in
  let dirty_files =
    Relative_path.Set.filter dirty_files ~f:FindUtils.path_filter
  in
  (*
    Tracked targets are build files that are tracked by version control.
    We don't need to typecheck them, but we do need to parse them to load
    them into memory, since arc rebuild deletes them before running.
    This avoids build step dependencies and file_heap_stale errors crashing
    the server when build fails and the deleted files aren't properly
    regenerated.
  *)
  let parsing_files =
    Relative_path.Set.union dirty_files tracked_targets in
  let parsing_files_list = Relative_path.Set.elements parsing_files in
  let old_fast = FileInfo.saved_to_fast old_saved in

  (* Get only the hack files for global naming *)
  let old_hack_files = FileInfo.saved_to_hack_files old_saved in
  let old_info = FileInfo.saved_to_info old_saved in
  (* Parse dirty files only *)
  let next = MultiWorker.next genv.workers parsing_files_list in
  let env, t = parsing genv env ~lazy_parse:true ~get_next:next
      ~count:(List.length parsing_files_list) t ~trace in
  SearchServiceRunner.update_fileinfo_map env.files_info;

  let t = update_files genv env.files_info t in
  (* Name all the files from the old fast (except the new ones we parsed) *)
  let old_hack_names = Relative_path.Map.filter old_hack_files (fun k _v ->
      not (Relative_path.Set.mem parsing_files k)
    ) in

  let t = naming_with_fast old_hack_names t in
  (* Do global naming on all dirty files *)
  let env, t = naming env t in

  (* Add all files from fast to the files_info object *)
  let fast = FileInfo.simplify_fast env.files_info in
  let failed_parsing = Errors.get_failed_files env.errorl Errors.Parsing in
  let fast = Relative_path.Set.fold failed_parsing
      ~f:(fun x m -> Relative_path.Map.remove m x) ~init:fast in

  let env = { env with
              disk_needs_parsing =
                Relative_path.Set.union env.disk_needs_parsing changed_while_parsing;
            } in

  (* Separate the dirty files from the files whose decl only changed *)
  (* Here, for each dirty file, we compare its hash to the one saved
     in the saved state. If the hashes are the same, then the declarations
     on the file have not changed and we only need to retypecheck that file,
     not all of its dependencies.
     We call these files "similar" to their previous versions. *)
  let partition_similar dirty_files = Relative_path.Set.partition
      (fun f ->
         let info1 = Relative_path.Map.get old_info f in
         let info2 = Relative_path.Map.get env.files_info f in
         match info1, info2 with
         | Some x, Some y ->
           (match x.FileInfo.hash, y.FileInfo.hash with
            | Some x, Some y ->
              OpaqueDigest.equal x y
            | _ ->
              false)
         | _ ->
           false
      ) dirty_files in

  let similar_master_files, dirty_master_files =
    partition_similar dirty_master_files in
  let similar_local_files, dirty_local_files =
    partition_similar dirty_local_files in

  let similar_files =
    Relative_path.Set.union similar_master_files similar_local_files in

  let env = { env with
              files_info=Relative_path.Map.union env.files_info old_info;
              (* The only reason old_parsing_error_files are added to disk_needs_parsing
                 here is because of an issue that seems to be already tracked in T30786759 *)
              disk_needs_parsing = old_parsing_error_files;
              needs_recheck = Relative_path.Set.union env.needs_recheck decl_and_typing_error_files;
            } in
  (* Update the fileinfo object's dependencies now that we have full fast *)
  let t = update_files genv env.files_info t in

  let t = update_search genv old_saved t in

  type_check_dirty genv env old_fast fast
    dirty_master_files dirty_local_files similar_files t

let init
    ~(load_state_approach: load_state_approach option)
    (genv: ServerEnv.genv)
    (lazy_level: lazy_level)
    (env: ServerEnv.env)
    (root: Path.t)
  : (ServerEnv.env * float) * (loaded_info * Relative_path.Set.t, error) result =
  assert(lazy_level = Init);
  ServerProgress.send_progress_to_monitor "loading saved state";

  (* A historical quirk: we allowed the timeout once while downloading+loading *)
  (* saved-state, and then once again while waiting to get dirty files from hg *)
  let timeout = 2 * genv.local_config.SLC.load_state_script_timeout in

  (* following function will be run under the timeout *)
  let do_ (_id: Timeout.t) : (loaded_info, error) result =
    match load_state_approach with
    | None ->
      Error Lazy_init_no_load_approach
    | Some (Precomputed info) ->
      Ok (use_precomputed_state_exn genv info)
    | Some (Load_state_natively use_canary) ->
      download_and_load_state_exn ~use_canary ~target:None ~genv ~root
    | Some (Load_state_natively_with_target target) ->
      download_and_load_state_exn ~use_canary: false ~target:(Some target) ~genv ~root in

  let state_result = try
      match Timeout.with_timeout ~timeout ~do_ ~on_timeout:(fun () -> Error Lazy_init_timeout) with
      | Error error ->
        Error error
      | Ok loaded_info ->
        let changed_while_parsing = get_updates_exn genv root in
        Ok (loaded_info, changed_while_parsing)
    with exn ->
      let stack = Utils.Callstack (Printexc.get_backtrace ()) in
      Error (Lazy_init_unhandled_exception {exn; stack;}) in

  match state_result with
  | Error error ->
    (* Fall back to type-checking everything *)
    fallback_init genv env error, state_result
  | Ok (loaded_info, changed_while_parsing) ->
    ServerProgress.send_progress_to_monitor "loading saved state succeeded";
    post_saved_state_initialization ~loaded_info ~changed_while_parsing ~env ~genv, state_result
