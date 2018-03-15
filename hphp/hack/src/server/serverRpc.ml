(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core
open ServerEnv
open ServerCommandTypes
open Utils

let remove_dead_fixme_warning = (
  "hh_server was started without '--no-load', which is required when removing dead fixmes.\n" ^
  "Please run 'hh_client restart --no-load' to restart it."
)

let handle : type a. genv -> env -> is_stale:bool -> a t -> env * a =
  fun genv env ~is_stale -> function
    | STATUS _ ->
        HackEventLogger.check_response (Errors.get_error_list env.errorl);
        let error_list = Errors.get_sorted_error_list env.errorl in
        let error_list = List.map ~f:Errors.to_absolute error_list in
        let liveness = (if is_stale then Stale_status else Live_status) in
        let has_unsaved_changes = ServerFileSync.has_unsaved_changes env in
        env, { Server_status.liveness; has_unsaved_changes; error_list; }
    | COVERAGE_LEVELS fn -> env, ServerColorFile.go env fn
    | INFER_TYPE (fn, line, char, dynamic_view) ->
        env, ServerInferType.go env (fn, line, char, dynamic_view)
    | INFER_TYPE_BATCH (positions, dynamic_view) ->
        let tcopt = env.ServerEnv.tcopt in
        let tcopt = { tcopt with GlobalOptions.tco_dynamic_view=dynamic_view } in
        let env = { env with tcopt } in
        env, ServerInferTypeBatch.go genv.workers positions env
    | IDE_HOVER (fn, line, char) ->
        env, ServerHover.go env (fn, line, char)
    | AUTOCOMPLETE content ->
        let result = try
          let autocomplete_context = { AutocompleteTypes.
            is_xhp_classname = false;
            is_instance_member = false;
          } in (* feature not implemented here; it only works for LSP *)
          ServerAutoComplete.auto_complete
            ~tcopt:env.tcopt ~delimit_on_namespaces:false ~autocomplete_context content
          with Decl.Decl_not_found s ->
            let s = s ^ "-- Autocomplete File contents: " ^ content in
            Printexc.print_backtrace stderr;
            raise (Decl.Decl_not_found s)
        in
        env, result.With_complete_flag.value
    | IDENTIFY_FUNCTION (file_input, line, char) ->
        let content = ServerFileSync.get_file_content file_input in
        env, ServerIdentifyFunction.go_absolute content line char env.tcopt
    | METHOD_JUMP (class_, filter, find_children) ->
      env, MethodJumps.get_inheritance env.tcopt class_ ~filter ~find_children
        env.files_info genv.workers
    | FIND_DEPENDENT_FILES file_list ->
        env, Ai.ServerFindDepFiles.go genv.workers file_list
          (ServerArgs.ai_mode genv.options)
    | FIND_REFS find_refs_action ->
        if ServerArgs.ai_mode genv.options = None then
          let include_defs = false in
          env, ServerFindRefs.go find_refs_action include_defs genv env
        else
          env, Ai.ServerFindRefs.go find_refs_action genv env
    | IDE_FIND_REFS (input, line, char, include_defs) ->
        let content = ServerFileSync.get_file_content input in
        let args = (content, line, char, include_defs) in
        let results = ServerFindRefs.go_from_file args genv env in
        env, results
    | IDE_HIGHLIGHT_REFS (input, line, char) ->
        let content = ServerFileSync.get_file_content input in
        env, ServerHighlightRefs.go (content, line, char) env.tcopt
    | REFACTOR refactor_action ->
        env, ServerRefactor.go refactor_action genv env
    | IDE_REFACTOR (file_input, line, char, new_name) ->
        let content = ServerFileSync.get_file_content file_input in
        let definitions = ServerIdentifyFunction.go_absolute content line char env.tcopt in
        env, ServerRefactor.go_ide definitions new_name genv env
    | REMOVE_DEAD_FIXMES codes ->
      if genv.ServerEnv.options |> ServerArgs.no_load then begin
        HackEventLogger.check_response (Errors.get_error_list env.errorl);
        env, `Ok (ServerRefactor.get_fixme_patches codes env)
      end else
        env, (`Error remove_dead_fixme_warning)
    | IGNORE_FIXMES files ->
      let paths = List.map files Relative_path.from_root in
      let disk_needs_parsing =
        List.fold_left
          paths
          ~init:env.disk_needs_parsing
          ~f:Relative_path.Set.add
      in
      Errors.set_ignored_fixmes (Some paths);
      let original_env = env in
      let env = {env with disk_needs_parsing} in
      (* Everything should happen on the master process *)
      let genv = {genv with workers = None} in
      let env, _, _ = ServerTypeCheck.(check genv env Full_check) in
      let error_list = Errors.get_sorted_error_list env.errorl in
      let error_list = List.map ~f:Errors.to_absolute error_list in
      Errors.set_ignored_fixmes None;
      let has_unsaved_changes = ServerFileSync.has_unsaved_changes env in
      original_env, { Ignore_fixmes_result.has_unsaved_changes; error_list; }
    | DUMP_SYMBOL_INFO file_list ->
        env, SymbolInfoService.go genv.workers file_list env
    | DUMP_AI_INFO file_list ->
        env, Ai.InfoService.go Typing_check_utils.check_defs genv.workers
          file_list (ServerArgs.ai_mode genv.options) env.tcopt
    | SEARCH (query, type_) ->
        env, ServerSearch.go env.tcopt genv.workers query type_
    | COVERAGE_COUNTS path -> env, ServerCoverageMetric.go path genv env
    | LINT fnl -> env, ServerLint.go genv env fnl
    | LINT_STDIN { filename; contents } ->
        env, ServerLint.go_stdin env ~filename ~contents
    | LINT_ALL code -> env, ServerLint.lint_all genv env code
    | CREATE_CHECKPOINT x -> env, ServerCheckpoint.create_checkpoint x
    | RETRIEVE_CHECKPOINT x -> env, ServerCheckpoint.retrieve_checkpoint x
    | DELETE_CHECKPOINT x -> env, ServerCheckpoint.delete_checkpoint x
    | STATS -> env, Stats.get_stats ()
    | KILL -> env, ()
    | FORMAT (content, from, to_) ->
        env, ServerFormat.go genv content from to_
    | IDE_FORMAT action ->
        env, ServerFormat.go_ide genv action
    | TRACE_AI action ->
        env, Ai.TraceService.go action Typing_check_utils.check_defs
           (ServerArgs.ai_mode genv.options) env.tcopt
    | AI_QUERY json ->
        env, Ai.QueryService.go json
    | DUMP_FULL_FIDELITY_PARSE file ->
        env, FullFidelityParseService.go file
    | OPEN_FILE (path, contents) ->
        ServerFileSync.open_file env path contents, ()
    | CLOSE_FILE path ->
        ServerFileSync.close_file env path, ()
    | EDIT_FILE (path, edits) ->
        let edits = List.map edits ~f:Ide_api_types.ide_text_edit_to_fc in
        ServerFileSync.edit_file env path edits, ()
    | IDE_AUTOCOMPLETE (path, pos, delimit_on_namespaces) ->
        let open File_content in
        let open With_complete_flag in
        let pos = pos |> Ide_api_types.ide_pos_to_fc in
        let fc = ServerFileSync.get_file_content (ServerUtils.FileName path) in
        let offset = File_content.get_offset fc pos in (* will raise if out of bounds *)
        let char_at_pos = File_content.get_char fc offset in
        let autocomplete_context = ServerAutoComplete.get_autocomplete_context fc pos in
        let edits = [{range = Some {st = pos; ed = pos}; text = "AUTO332"}] in
        let content = File_content.edit_file_unsafe fc edits in
        let results = ServerAutoComplete.auto_complete
          ~tcopt:env.tcopt ~delimit_on_namespaces ~autocomplete_context content in
        let completions = results.value in
        let is_complete = results.is_complete in
        env, { AutocompleteTypes.completions; char_at_pos; is_complete; }
    | IDE_FFP_AUTOCOMPLETE (path, pos) ->
        let pos = pos |> Ide_api_types.ide_pos_to_fc in
        let content = ServerFileSync.get_file_content (ServerUtils.FileName path) in
        let offset = File_content.get_offset content pos in (* will raise if out of bounds *)
        let char_at_pos = File_content.get_char content offset in
        let result =
          FfpAutocompleteService.auto_complete env.tcopt content pos ~filter_by_token:false
        in
        env, { AutocompleteTypes.completions = result; char_at_pos; is_complete = true; }
    | DISCONNECT ->
        ServerFileSync.clear_sync_data env, ()
    | SUBSCRIBE_DIAGNOSTIC id ->
        let new_env = { env with
          diag_subscribe = Some (Diagnostic_subscription.of_id id env.errorl)
        } in
        new_env, ()
    | UNSUBSCRIBE_DIAGNOSTIC id ->
        let diag_subscribe = match env.diag_subscribe with
          | Some x when Diagnostic_subscription.get_id x = id -> None
          | x -> x
        in
        let new_env = { env with diag_subscribe } in
        new_env, ()
    | OUTLINE path ->
      env, ServerUtils.FileName path |>
      ServerFileSync.get_file_content |>
      FileOutline.outline env.popt
    | IDE_IDLE ->
      {env with ide_idle = true;}, ()
    | RAGE ->
      env, ServerRage.go genv env
    | INFER_RETURN_TYPE id_info ->
      match id_info with
      | InferReturnTypeService.Function fun_name ->
        env, InferReturnTypeService.get_fun_return_ty
          env.tcopt env.popt fun_name
      | InferReturnTypeService.Method (class_name, meth_name) ->
        env, InferReturnTypeService.get_meth_return_ty
          env.tcopt env.popt class_name meth_name
