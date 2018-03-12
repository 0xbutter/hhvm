(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Hh_core

module SU = Hhbc_string_utils

(* TODO: Remove this once we start reading this off of HHVM's config.hdf *)
let default_auto_aliased_namespaces = [
  "Arrays", "HH\\Lib\\Arrays"
  ; "C", "HH\\Lib\\C"
  ; "Dict", "HH\\Lib\\Dict"
  ; "Keyset", "HH\\Lib\\Keyset"
  ; "Math", "HH\\Lib\\Math"
  ; "PHP", "HH\\Lib\\PHP"
  ; "Str", "HH\\Lib\\Str"
  ; "Vec", "HH\\Lib\\Vec"
  ]
let auto_namespace_map () =
  Option.value Hhbc_options.(aliased_namespaces !compiler_options)
    ~default:default_auto_aliased_namespaces

let should_alias_name ns id aliased_id =
  (* Take any "use namespace" clauses (but not just use clauses) *)
  (* A use clause also fills the use_class list, where as a
    use namespace clause only fills the first list. Thus we
    are looking for namespaces in the first list but not the second
  *)
  let use_ns = SMap.keys ns.Namespace_env.ns_ns_uses in
  let use_class = SMap.keys ns.Namespace_env.ns_class_uses in
  let in_ns l =
    List.exists l (String_utils.string_starts_with id)  in
  aliased_id <> id
  (* If we expicitly used the namespace, this overrides the namespace aliasing *)
  && not (in_ns use_ns && not (in_ns use_class))

let elaborate_id ~should_alias ns kind id =
  let autoimport =
    if List.mem
      Hh_autoimport.autoimport_only_for_typechecker (SU.strip_ns @@ snd id)
    then false else Emit_env.is_hh_syntax_enabled () in

  let aliased_id =
    Namespaces.renamespace_if_aliased
      ~reverse:true (auto_namespace_map ()) (snd id) in
  let was_renamed, fully_qualified_id =
    if should_alias && should_alias_name ns (snd id) aliased_id
    then false, aliased_id
    else
      let was_renamed, (_, fully_qualified_id) =
        Namespaces.elaborate_id_impl ~autoimport ns kind id in
      was_renamed, fully_qualified_id
    in

  let stripped_fully_qualified_id = SU.strip_global_ns fully_qualified_id in
  let clean_id = SU.strip_ns fully_qualified_id in
  let need_fallback =
    stripped_fully_qualified_id <> clean_id &&
    (String.contains stripped_fully_qualified_id '\\') &&
    not (String.contains (snd id) '\\')
  in
  was_renamed, stripped_fully_qualified_id, if need_fallback then Some clean_id else None

(* Class identifier, with namespace qualification if not global, but without
 * initial backslash.
 *)
module Class = struct
  type t = string

  let from_ast_name s =
    Hh_autoimport.normalize
      ~is_hack:(Emit_env.is_hh_syntax_enabled ())
      ~php7_scalar_types:(Hhbc_options.php7_scalar_types !Hhbc_options.compiler_options)
      (SU.strip_global_ns s)
  let from_raw_string s = s
  let to_raw_string s = s
  let elaborate_id_ ~should_alias ns ((_, n) as id) =
    let ns =
      if SU.Xhp.is_xhp n
      then Namespace_env.empty ns.Namespace_env.ns_popt
      else ns in
    let mangled_name = SU.Xhp.mangle n in
    let stripped_mangled_name = SU.strip_global_ns mangled_name in
    let was_renamed, id, fallback_id =
      elaborate_id ~should_alias ns Namespaces.ElaborateClass (fst id, mangled_name) in
    if was_renamed || mangled_name.[0] = '\\'
    then id, fallback_id
    else
    match Hh_autoimport.opt_normalize
      ~is_hack:(Emit_env.is_hh_syntax_enabled ())
      ~php7_scalar_types:(Hhbc_options.php7_scalar_types !Hhbc_options.compiler_options)
      stripped_mangled_name with
    | None -> id, fallback_id
    | Some s -> s, None

  let elaborate_id_at_definition_site = elaborate_id_ ~should_alias:false
  let elaborate_id = elaborate_id_ ~should_alias:true
  let to_unmangled_string s =
    SU.Xhp.unmangle s
end

module Prop = struct
  type t = string

  let from_raw_string s = s
  let from_ast_name s = SU.strip_global_ns s
  let add_suffix s suffix = s ^ suffix
  let to_raw_string s = s
end

module Method = struct
  type t = string

  let from_raw_string s = s
  let from_ast_name s = SU.strip_global_ns s
  let add_suffix s suffix = s ^ suffix
  let to_raw_string s = s
end

module Function = struct
  type t = string

  (* See hphp/compiler/parser.cpp. *)
  let builtins_in_hh =
  [
    "fun";
    "meth_caller";
    "class_meth";
    "inst_meth";
    "invariant_callback_register";
    "invariant";
    "invariant_violation";
    "idx";
    "type_structure";
    "asio_get_current_context_idx";
    "asio_get_running_in_context";
    "asio_get_running";
    "xenon_get_data";
    "thread_memory_stats";
    "thread_mark_stack";
    "objprof_get_strings";
    "objprof_get_data";
    "objprof_get_paths";
    "heapgraph_create";
    "heapgraph_stats";
    "heapgraph_foreach_node";
    "heapgraph_foreach_edge";
    "heapgraph_foreach_root";
    "heapgraph_dfs_nodes";
    "heapgraph_dfs_edges";
    "heapgraph_node";
    "heapgraph_edge";
    "heapgraph_node_in_edges";
    "heapgraph_node_out_edges";
    "server_warmup_status";
    "dict";
    "vec";
    "keyset";
    "varray";
    "darray";
    "is_vec";
    "is_dict";
    "is_keyset";
    "is_varray";
    "is_darray";
  ]

  let builtins_at_top = [
    "echo";
    "exit";
    "die";
    "func_get_args";
    "func_get_arg";
    "func_num_args"
  ]

  let has_hh_prefix s =
    let s = String.lowercase_ascii s in
    String_utils.string_starts_with s "hh\\"

  let is_hh_builtin s =
    let s = if has_hh_prefix s then String_utils.lstrip s "hh\\" else s in
    List.mem builtins_in_hh s

  let from_raw_string s = s
  let to_raw_string s = s
  let add_suffix s suffix = s ^ suffix

  let elaborate_id_ ~should_alias ns id =
    let _, x, y = elaborate_id ~should_alias ns Namespaces.ElaborateFun id in
    x, y

  let elaborate_id_at_definition_site = elaborate_id_ ~should_alias:false
  let elaborate_id = elaborate_id_ ~should_alias:true
  let elaborate_id_with_builtins ns id =
    let fq_id, backoff_id = elaborate_id ns id in
    match backoff_id with
      (* OK we are in a namespace so let's look at the backoff ID and see if
       * it's an HH\ or top-level function with implicit namespace.
       *)
    | Some id ->
      if List.mem builtins_in_hh id && (Emit_env.is_hh_syntax_enabled ())
      then SU.prefix_namespace "HH" id, Some id
      else if List.mem builtins_at_top id
      then id, None
      else fq_id, backoff_id
      (* Likewise for top-level, with no namespace *)
    | None ->
      if is_hh_builtin fq_id && (Emit_env.is_hh_syntax_enabled ())
      then
        if has_hh_prefix fq_id
        then fq_id, None
        else SU.prefix_namespace "HH" fq_id, Some fq_id
      else fq_id, None
end

module Const = struct
  type t = string

  let from_ast_name s = SU.strip_global_ns s
  let from_raw_string s = s
  let to_raw_string s = s
  let elaborate_id ns id =
    let _was_renamed, fq_id, backoff_id = elaborate_id ~should_alias:true ns Namespaces.ElaborateConst id in
    fq_id, backoff_id, String.contains (snd id) '\\'

end
