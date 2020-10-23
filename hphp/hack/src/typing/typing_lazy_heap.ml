(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let get_type_id_filename ctx x expected_kind =
  match Naming_provider.get_type_path_and_kind ctx x with
  | Some (fn, kind) when Naming_types.equal_kind_of_type kind expected_kind ->
    Some fn
  | _ -> None

let get_class = Typing_classes_heap.Classes.get

let get_class_no_local_cache = Typing_classes_heap.Classes.get_no_local_cache

let get_fun ~(sh : SharedMem.uses) ctx x =
  Decl_counters.count_decl x Decl_counters.Fun @@ fun () ->
  let SharedMem.Uses = sh in
  match Typing_heap.Funs.get x with
  | Some c -> Some c
  | None ->
    (match Naming_provider.get_fun_path ctx x with
    | Some filename ->
      let ft =
        Errors.run_in_decl_mode filename (fun () ->
            Decl.declare_fun_in_file ~write_shmem:true ctx filename x)
      in
      Some ft
    | None -> None)

let get_gconst ~(sh : SharedMem.uses) ctx cst_name =
  Decl_counters.count_decl cst_name Decl_counters.GConst @@ fun () ->
  let SharedMem.Uses = sh in
  match Typing_heap.GConsts.get cst_name with
  | Some c -> Some c
  | None ->
    (match Naming_provider.get_const_path ctx cst_name with
    | Some filename ->
      let gconst =
        Errors.run_in_decl_mode filename (fun () ->
            Decl.declare_const_in_file ~write_shmem:true ctx filename cst_name)
      in
      Some gconst
    | None -> None)

let get_record_def ~(sh : SharedMem.uses) ctx x =
  Decl_counters.count_decl x Decl_counters.Record_def @@ fun () ->
  let SharedMem.Uses = sh in
  match Typing_heap.RecordDefs.get x with
  | Some c -> Some c
  | None ->
    (match get_type_id_filename ctx x Naming_types.TRecordDef with
    | Some filename ->
      let tdecl =
        Errors.run_in_decl_mode filename (fun () ->
            Decl.declare_record_def_in_file ~write_shmem:true ctx filename x)
      in
      Some tdecl
    | None -> None)

let get_typedef ~(sh : SharedMem.uses) ctx x =
  Decl_counters.count_decl x Decl_counters.Typedef @@ fun () ->
  let SharedMem.Uses = sh in
  match Typing_heap.Typedefs.get x with
  | Some c -> Some c
  | None ->
    (match get_type_id_filename ctx x Naming_types.TTypedef with
    | Some filename ->
      let tdecl =
        Errors.run_in_decl_mode filename (fun () ->
            Decl.declare_typedef_in_file ~write_shmem:true ctx filename x)
      in
      Some tdecl
    | None -> None)
