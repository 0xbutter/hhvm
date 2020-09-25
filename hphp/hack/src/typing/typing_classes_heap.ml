(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Decl_inheritance
open Shallow_decl_defs
open Typing_defs
module Attrs = Typing_defs.Attributes
module LSTable = Lazy_string_table
module SN = Naming_special_names

type lazy_class_type = {
  sc: shallow_class;
  ih: inherited_members;
  ancestors: decl_ty LSTable.t;
  parents_and_traits: unit LSTable.t;
  members_fully_known: bool Lazy.t;
  req_ancestor_names: unit LSTable.t;
  all_requirements: (Pos.t * decl_ty) list Lazy.t;
}

type class_type_variant =
  | Lazy of lazy_class_type
  | Eager of class_type

let make_lazy_class_type ctx class_name =
  match Shallow_classes_provider.get ctx class_name with
  | None -> None
  | Some sc ->
    let Decl_ancestors.
          {
            ancestors;
            parents_and_traits;
            members_fully_known;
            req_ancestor_names;
            all_requirements;
          } =
      Decl_ancestors.make ctx class_name
    in
    let get_ancestor = LSTable.get ancestors in
    let inherited_members = Decl_inheritance.make ctx class_name get_ancestor in
    Some
      (Lazy
         {
           sc;
           ih = inherited_members;
           ancestors;
           parents_and_traits;
           members_fully_known;
           req_ancestor_names;
           all_requirements;
         })

let make_eager_class_type ctx class_name =
  match Decl_heap.Classes.get class_name with
  | Some decl -> Some (Eager (Decl_class.to_class_type decl))
  | None ->
    begin
      match Naming_provider.get_type_path_and_kind ctx class_name with
      | Some (_, Naming_types.TTypedef)
      | Some (_, Naming_types.TRecordDef)
      | None ->
        None
      | Some (file, Naming_types.TClass) ->
        Deferred_decl.raise_if_should_defer ~d:file;
        (* declare_class_in_file actual reads from Decl_heap.Classes.get
        like what we do above, which makes our test redundant but cleaner.
        It also writes into Decl_heap.Classes and other Decl_heaps. *)
        let decl =
          Errors.run_in_decl_mode file (fun () ->
              Decl.declare_class_in_file ~sh:SharedMem.Uses ctx file class_name)
        in
        Deferred_decl.increment_counter ();
        Some (Eager (Decl_class.to_class_type (Option.value_exn decl)))
    end

module Classes = struct
  (** This module is an abstraction layer over shallow vs folded decl,
      and provides a view of classes which includes all
      inherited members and their types.

      In legacy folded decl, that view is constructed by merging a single
      heap entry for the folded class with many entries for the types
      of each of its members (those member entries are looked up lazily,
      as needed).

      In shallow decl, that view is constructed even more lazily,
      by iterating over the shallow representation of the class
      and its ancestors one at a time. *)

  (** This cache caches the result of full class computations
      (the class merged with all its inherited members.)  *)
  module Cache =
    SharedMem.LocalCache
      (StringKey)
      (struct
        type t = class_type_variant

        let prefix = Prefix.make ()

        let description = "Decl_Typing_ClassType"
      end)
      (struct
        let capacity = 1000
      end)

  type key = StringKey.t

  type t = class_type_variant

  (** Fetches either the [Lazy] class (if shallow decls are enabled)
  or the [Eager] class (otherwise).
  Note: Eager will always read+write to shmem Decl_heaps. Lazy
  will solely go through the ctx provider. *)
  let compute_class_decl (ctx : Provider_context.t) (class_name : string) :
      t option =
    try
      if TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)
      then
        make_lazy_class_type ctx class_name
      else
        make_eager_class_type ctx class_name
    with Deferred_decl.Defer d ->
      Deferred_decl.add_deferment ~d;
      None

  (** This gets a class_type variant, via the local-memory [Cache]. *)
  let get ctx class_name =
    Counters.count_decl_accessor @@ fun () ->
    match Cache.get class_name with
    | Some t -> Some t
    | None ->
      begin
        match compute_class_decl ctx class_name with
        | None -> None
        | Some class_type_variant ->
          Cache.add class_name class_type_variant;
          Some class_type_variant
      end

  let find_unsafe ctx key =
    match get ctx key with
    | None -> raise Caml.Not_found
    | Some x -> x

  let mem ctx key =
    match get ctx key with
    | None -> false
    | Some _ -> true
end

module Api = struct
  type t = class_type_variant

  (** Internal helper, for all the accessors that return sequences,
  so we properly tally up the time spent iterating over those sequences. *)
  let tally_sequence (seq : 'a Sequence.t) : 'a Sequence.t =
    Sequence.map
      ~f:(fun elem -> Counters.count_decl_accessor (fun () -> elem))
      seq

  let members_fully_known t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> Lazy.force lc.members_fully_known
    | Eager c -> c.tc_members_fully_known

  let abstract t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc ->
      begin
        match lc.sc.sc_kind with
        | Ast_defs.Cabstract
        | Ast_defs.Cinterface
        | Ast_defs.Ctrait
        | Ast_defs.Cenum ->
          true
        | _ -> false
      end
    | Eager c -> c.tc_abstract

  let final t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> lc.sc.sc_final
    | Eager c -> c.tc_final

  let const t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> Attrs.mem SN.UserAttributes.uaConst lc.sc.sc_user_attributes
    | Eager c -> c.tc_const

  let kind t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> lc.sc.sc_kind
    | Eager c -> c.tc_kind

  let is_xhp t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> lc.sc.sc_is_xhp
    | Eager c -> c.tc_is_xhp

  let name t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> snd lc.sc.sc_name
    | Eager c -> c.tc_name

  let pos t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> fst lc.sc.sc_name
    | Eager c -> c.tc_pos

  let tparams t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> lc.sc.sc_tparams
    | Eager c -> c.tc_tparams

  let where_constraints t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> lc.sc.sc_where_constraints
    | Eager c -> c.tc_where_constraints

  let construct t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> Lazy.force lc.ih.construct
    | Eager c -> c.tc_construct

  let enum_type t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> lc.sc.sc_enum_type
    | Eager c -> c.tc_enum_type

  let sealed_whitelist t =
    Counters.count_decl_accessor @@ fun () ->
    let get_sealed_whitelist sc =
      match Attrs.find SN.UserAttributes.uaSealed sc.sc_user_attributes with
      | None -> None
      | Some { ua_classname_params; _ } ->
        Some (SSet.of_list ua_classname_params)
    in
    match t with
    | Lazy lc -> get_sealed_whitelist lc.sc
    | Eager c -> c.tc_sealed_whitelist

  let decl_errors t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> Some lc.sc.sc_decl_errors
    | Eager c -> c.tc_decl_errors

  let sort_by_key seq =
    (* No tally on this helper; all its callers do tally themselves. *)
    seq
    |> Sequence.to_list_rev
    |> List.sort ~compare:(fun (a, _) (b, _) -> String.compare a b)

  let get_ancestor t ancestor =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ancestors ancestor
    | Eager c -> SMap.find_opt ancestor c.tc_ancestors

  let has_ancestor t ancestor =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.ancestors ancestor
    | Eager c -> SMap.mem ancestor c.tc_ancestors

  let need_init t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy _ ->
      begin
        match fst (construct t) with
        | None -> false
        | Some ce -> not (get_ce_abstract ce)
      end
    | Eager c -> c.tc_need_init

  (* We cannot invoke [Typing_deferred_members.class_] here because it would be a
     dependency cycle. Instead, we raise an exception. We should remove this
     function (along with [Decl_init_check]) altogether when we delete legacy
     class declaration, since [Typing_deferred_members] makes it obsolete. *)
  let deferred_init_members t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy _ -> failwith "shallow_class_decl is enabled"
    | Eager c -> c.tc_deferred_init_members

  let requires_ancestor t ancestor =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.req_ancestor_names ancestor
    | Eager c -> SSet.mem ancestor c.tc_req_ancestors_extends

  let extends t ancestor =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.parents_and_traits ancestor
    | Eager c -> SSet.mem ancestor c.tc_extends

  let all_ancestors t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_seq lc.ancestors |> sort_by_key
    | Eager c -> SMap.bindings c.tc_ancestors

  let all_ancestor_names t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy _ -> List.map (all_ancestors t) fst
    | Eager c -> SMap.ordered_keys c.tc_ancestors

  let all_ancestor_reqs t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> Lazy.force lc.all_requirements
    | Eager c -> c.tc_req_ancestors

  let all_ancestor_req_names t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc ->
      LSTable.to_seq lc.req_ancestor_names
      |> Sequence.map ~f:fst
      |> tally_sequence
    | Eager c -> Sequence.of_list (SSet.elements c.tc_req_ancestors_extends)

  let all_extends_ancestors t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc ->
      LSTable.to_seq lc.parents_and_traits
      |> Sequence.map ~f:fst
      |> tally_sequence
    | Eager c -> Sequence.of_list (SSet.elements c.tc_extends)

  let all_where_constraints_on_this t =
    (* tally is already done by where_constraints *)
    List.filter
      ~f:(fun (l, _, r) ->
        match (get_node l, get_node r) with
        | (Tthis, _)
        | (_, Tthis) ->
          true
        | _ -> false)
      (where_constraints t)

  let upper_bounds_on_this_from_constraints t =
    (* tally is already done by where_constraints *)
    List.filter_map
      ~f:(fun (l, c, r) ->
        match (get_node l, c, get_node r) with
        | (Tthis, Ast_defs.Constraint_as, _)
        | (Tthis, Ast_defs.Constraint_eq, _) ->
          Some r
        | (_, Ast_defs.Constraint_eq, Tthis)
        | (_, Ast_defs.Constraint_super, Tthis) ->
          Some l
        | _ -> None)
      (where_constraints t)

  (* get upper bounds on `this` from the where constraints as well as
   * requirements *)
  let upper_bounds_on_this t =
    (* tally is already done by all_ancestors and upper_bounds *)
    List.map ~f:(fun req -> snd req) (all_ancestor_reqs t)
    |> List.append (upper_bounds_on_this_from_constraints t)

  let has_upper_bounds_on_this_from_constraints t =
    (* tally is already done by upper_bounds_on_this *)
    not (List.is_empty (upper_bounds_on_this_from_constraints t))

  (* get lower bounds on `this` from the where constraints *)
  let lower_bounds_on_this_from_constraints t =
    (* tally is already done by where_constraint *)
    List.filter_map
      ~f:(fun (l, c, r) ->
        match (get_node l, c, get_node r) with
        | (Tthis, Ast_defs.Constraint_super, _)
        | (Tthis, Ast_defs.Constraint_eq, _) ->
          Some r
        | (_, Ast_defs.Constraint_eq, Tthis)
        | (_, Ast_defs.Constraint_as, Tthis) ->
          Some l
        | _ -> None)
      (where_constraints t)

  let lower_bounds_on_this t =
    (* tally is done inside the following method *)
    lower_bounds_on_this_from_constraints t

  let has_lower_bounds_on_this_from_constraints t =
    (* tally is already done by lower_bounds_on_this *)
    not (List.is_empty (lower_bounds_on_this_from_constraints t))

  let is_disposable t =
    Counters.count_decl_accessor @@ fun () ->
    let is_disposable_class_name class_name =
      String.equal class_name SN.Classes.cIDisposable
      || String.equal class_name SN.Classes.cIAsyncDisposable
    in
    match t with
    | Lazy _ ->
      is_disposable_class_name (name t)
      || List.exists (all_ancestor_names t) is_disposable_class_name
      || Sequence.exists (all_ancestor_req_names t) is_disposable_class_name
    | Eager c -> c.tc_is_disposable

  let get_const t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ih.consts id
    | Eager c -> SMap.find_opt id c.tc_consts

  let get_typeconst t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ih.typeconsts id
    | Eager c -> SMap.find_opt id c.tc_typeconsts

  let get_pu_enum t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ih.pu_enums id
    | Eager c -> SMap.find_opt id c.tc_pu_enums

  let get_prop t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ih.props id
    | Eager c -> SMap.find_opt id c.tc_props

  let get_sprop t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ih.sprops id
    | Eager c -> SMap.find_opt id c.tc_sprops

  let get_method t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ih.methods id
    | Eager c -> SMap.find_opt id c.tc_methods

  let get_smethod t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.get lc.ih.smethods id
    | Eager c -> SMap.find_opt id c.tc_smethods

  let get_any_method ~is_static cls id =
    (* tally is already done inside the following three methods *)
    if String.equal id SN.Members.__construct then
      fst (construct cls)
    else if is_static then
      get_smethod cls id
    else
      get_method cls id

  let has_const t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.ih.consts id
    | Eager _ -> Option.is_some (get_const t id)

  let has_typeconst t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.ih.typeconsts id
    | Eager _ -> Option.is_some (get_typeconst t id)

  let has_prop t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.ih.props id
    | Eager _ -> Option.is_some (get_prop t id)

  let has_sprop t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.ih.sprops id
    | Eager _ -> Option.is_some (get_sprop t id)

  let has_method t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.ih.methods id
    | Eager _ -> Option.is_some (get_method t id)

  let has_smethod t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.mem lc.ih.smethods id
    | Eager _ -> Option.is_some (get_smethod t id)

  let consts t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_seq lc.ih.consts |> sort_by_key
    | Eager c -> SMap.bindings c.tc_consts

  let typeconsts t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_seq lc.ih.typeconsts |> sort_by_key
    | Eager c -> SMap.bindings c.tc_typeconsts

  let pu_enums t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_seq lc.ih.pu_enums |> sort_by_key
    | Eager c -> SMap.bindings c.tc_pu_enums

  let props t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_seq lc.ih.props |> sort_by_key
    | Eager c -> SMap.bindings c.tc_props

  let sprops t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_seq lc.ih.sprops |> sort_by_key
    | Eager c -> SMap.bindings c.tc_sprops

  let methods t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_seq lc.ih.methods |> sort_by_key
    | Eager c -> SMap.bindings c.tc_methods

  let smethods t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> LSTable.to_seq lc.ih.smethods |> sort_by_key
    | Eager c -> SMap.bindings c.tc_smethods

  let get_all cache id =
    (* tally: not applicable to this helper, called by other things that themselves tally *)
    LSTable.get cache id |> Option.value ~default:[]

  let all_inherited_methods t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> get_all lc.ih.all_inherited_methods id
    | Eager _ -> failwith "shallow_class_decl is disabled"

  let all_inherited_smethods t id =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> get_all lc.ih.all_inherited_smethods id
    | Eager _ -> failwith "shallow_class_decl is disabled"

  let shallow_decl t =
    Counters.count_decl_accessor @@ fun () ->
    match t with
    | Lazy lc -> lc.sc
    | Eager _ -> failwith "shallow_class_decl is disabled"
end

let compute_class_decl_no_cache = Classes.compute_class_decl
