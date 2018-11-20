(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

type t = class_type

module Classes = struct
  module Cache = SharedMem.LocalCache (StringKey) (struct
    type t = class_type
    let prefix = Prefix.make()
    let description = "ClassType"
    let use_sqlite_fallback () = false
  end)

  type key = StringKey.t
  type t = class_type

  let get key =
    match Cache.get key with
    | Some c -> Some c
    | None ->
      match Decl_heap.Classes.get key with
      | Some c ->
        let class_type = Decl_class.to_class_type c in
        Cache.add key class_type;
        Some class_type
      | None ->
        None

  let find_unsafe key =
    match get key with
    | None -> raise Caml.Not_found
    | Some x -> x

  let mem key =
    match get key with
    | None -> false
    | Some _ -> true
end

let need_init cls = cls.tc_need_init
let members_fully_known cls = cls.tc_members_fully_known
let abstract cls = cls.tc_abstract
let final cls = cls.tc_final
let const cls = cls.tc_const
let deferred_init_members cls = cls.tc_deferred_init_members
let kind cls = cls.tc_kind
let is_xhp cls = cls.tc_is_xhp
let is_disposable cls = cls.tc_is_disposable
let name cls = cls.tc_name
let pos cls = cls.tc_pos
let tparams cls = cls.tc_tparams
let consts cls = cls.tc_consts
let typeconsts cls = cls.tc_typeconsts
let props cls = cls.tc_props
let sprops cls = cls.tc_sprops
let methods cls = cls.tc_methods
let smethods cls = cls.tc_smethods
let construct cls = cls.tc_construct
let ancestors cls = cls.tc_ancestors
let req_ancestors cls = cls.tc_req_ancestors
let req_ancestors_extends cls = cls.tc_req_ancestors_extends
let extends cls = cls.tc_extends
let enum_type cls = cls.tc_enum_type
let decl_errors cls = cls.tc_decl_errors
