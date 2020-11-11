(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let cache_decls ctx decls =
  let open Shallow_decl_defs in
  let open Typing_defs in
  match Provider_context.get_backend ctx with
  | Provider_backend.Shared_memory ->
    List.iter decls ~f:(function
        | (name, Class decl) -> Shallow_classes_heap.Classes.add name decl
        | (name, Fun decl) -> Decl_heap.Funs.add name decl
        | (name, Record decl) -> Decl_heap.RecordDefs.add name decl
        | (name, Typedef decl) -> Decl_heap.Typedefs.add name decl
        | (name, Const decl) -> Decl_heap.GConsts.add name decl.cd_type)
  | Provider_backend.(Local_memory { decl_cache; shallow_decl_cache; _ }) ->
    List.iter decls ~f:(function
        | (name, Class decl) ->
          let (_ : shallow_class option) =
            Provider_backend.Shallow_decl_cache.find_or_add
              shallow_decl_cache
              ~key:
                (Provider_backend.Shallow_decl_cache_entry.Shallow_class_decl
                   name)
              ~default:(fun () -> Some decl)
          in
          ()
        | (name, Fun decl) ->
          let (_ : fun_elt option) =
            Provider_backend.Decl_cache.find_or_add
              decl_cache
              ~key:(Provider_backend.Decl_cache_entry.Fun_decl name)
              ~default:(fun () -> Some decl)
          in
          ()
        | (name, Record decl) ->
          let (_ : record_def_type option) =
            Provider_backend.Decl_cache.find_or_add
              decl_cache
              ~key:(Provider_backend.Decl_cache_entry.Record_decl name)
              ~default:(fun () -> Some decl)
          in
          ()
        | (name, Typedef decl) ->
          let (_ : typedef_type option) =
            Provider_backend.Decl_cache.find_or_add
              decl_cache
              ~key:(Provider_backend.Decl_cache_entry.Typedef_decl name)
              ~default:(fun () -> Some decl)
          in
          ()
        | (name, Const decl) ->
          let (_ : decl_ty option) =
            Provider_backend.Decl_cache.find_or_add
              decl_cache
              ~key:(Provider_backend.Decl_cache_entry.Gconst_decl name)
              ~default:(fun () -> Some decl.cd_type)
          in
          ())
  | Provider_backend.Decl_service _ ->
    failwith
      "Direct_decl_utils.cache_file_decls not implemented for Decl_service"

let get_file_contents ctx filename =
  match
    Relative_path.Map.find_opt (Provider_context.get_entries ctx) filename
  with
  | Some entry ->
    let source_text = Ast_provider.compute_source_text entry in
    Some (Full_fidelity_source_text.text source_text)
  | None -> File_provider.get_contents filename

let direct_decl_parse_and_cache ctx file =
  match get_file_contents ctx file with
  | None -> None
  | Some contents ->
    let popt = Provider_context.get_popt ctx in
    let ns_map = ParserOptions.auto_namespace_map popt in
    let decls = Direct_decl_parser.parse_decls_ffi file contents ns_map in
    cache_decls ctx decls;
    Some decls
