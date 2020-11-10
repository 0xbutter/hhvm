(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let shallow_decl_enabled (ctx : Provider_context.t) : bool =
  TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)

let class_decl_if_missing
    ~(sh : SharedMem.uses) (ctx : Provider_context.t) (c : Nast.class_) : unit =
  if shallow_decl_enabled ctx then
    let (_ : Shallow_decl_defs.shallow_class) =
      Shallow_classes_provider.decl ctx ~use_cache:true c
    in
    ()
  else
    let (_ : _ * _) = Decl_folded_class.class_decl_if_missing ~sh ctx c in
    ()

let rec name_and_declare_types_program
    ~(sh : SharedMem.uses) (ctx : Provider_context.t) (prog : Nast.program) :
    unit =
  let open Aast in
  List.iter prog (fun def ->
      match def with
      | Namespace (_, prog) -> name_and_declare_types_program ~sh ctx prog
      | NamespaceUse _ -> ()
      | SetNamespaceEnv _ -> ()
      | FileAttributes _ -> ()
      | Fun f ->
        let (name, decl) = Decl_nast.fun_naming_and_decl ctx f in
        Decl_heap.Funs.add name decl
      | Class c -> class_decl_if_missing ~sh ctx c
      | RecordDef rd ->
        let (name, decl) = Decl_nast.record_def_naming_and_decl ctx rd in
        Decl_heap.RecordDefs.add name decl
      | Typedef typedef ->
        let (name, decl) = Decl_nast.typedef_naming_and_decl ctx typedef in
        Decl_heap.Typedefs.add name decl
      | Stmt _ -> ()
      | Constant cst ->
        let (name, decl) = Decl_nast.const_naming_and_decl ctx cst in
        Decl_heap.GConsts.add name decl)

let make_env
    ~(sh : SharedMem.uses) (ctx : Provider_context.t) (fn : Relative_path.t) :
    unit =
  let ast = Ast_provider.get_ast ctx fn in
  name_and_declare_types_program ~sh ctx ast;
  ()

let err_not_found (file : Relative_path.t) (name : string) : 'a =
  let err_str =
    Printf.sprintf "%s not found in %s" name (Relative_path.to_absolute file)
  in
  raise (Decl_defs.Decl_not_found err_str)

let declare_folded_class_in_file
    ~(sh : SharedMem.uses)
    (ctx : Provider_context.t)
    (file : Relative_path.t)
    (name : string) : Decl_defs.decl_class_type * Decl_heap.class_members option
    =
  match Ast_provider.find_class_in_file ctx file name with
  | Some cls ->
    let (_name, decl) = Decl_folded_class.class_decl_if_missing ~sh ctx cls in
    decl
  | None -> err_not_found file name
