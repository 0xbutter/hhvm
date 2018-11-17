(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Ast_defs
open Aast_defs
open Typing_defs

module Env = Tast_env
module TMT = Typing_make_type

(** Return true if ty definitely does not contain null.  I.e., the
    return value false can mean two things: ty does contain null, e.g.,
    it is an option type; or we cannot tell, e.g., it is an opaque
    newtype. *)
let rec type_non_nullable env ty =
  let _, ty = Env.expand_type env ty in
  match ty with
  | _, (Tprim Nast.(Tint | Tbool | Tfloat | Tstring | Tresource | Tnum
                    | Tarraykey | Tnoreturn)
        | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tanon _ | Tobject
        | Tclass _ | Tarraykind _ | Tabstract (AKenum _, _)) -> true
  | _, Tabstract (_, Some ty) when type_non_nullable env ty -> true
  | _, Tunresolved tyl when not (List.is_empty tyl) ->
    List.for_all tyl (type_non_nullable env)
  | _ -> false

(* Truthiness utilities ******************************************************)
(* For types other than bool used in condition expressions, boolean binary
   expressions, and casts to bool. An always-truthy type used in a condition is
   always a logic error. A nullable possibly-falsy type used in a condition is a
   sketchy null check--it isn't clear whether the user meant to check for null
   or for one of the other falsy values. *)

type truthiness =
  | Unknown
  | Always_truthy
  | Always_falsy
  | Possibly_falsy

let fold_truthiness acc truthiness =
  match acc, truthiness with
  | Unknown, _ | _, Unknown -> Unknown

  | Always_truthy,  Always_truthy  -> Always_truthy
  | Always_falsy,   Always_falsy   -> Always_falsy
  | Possibly_falsy, Possibly_falsy -> Possibly_falsy

  | _ -> Possibly_falsy

let tclass_is_falsy_when_empty, is_traversable =
  let r = Typing_reason.Rnone in
  let simple_xml_el = TMT.class_type r "\\SimpleXMLElement" [] in
  let container_type = TMT.container r (r, Tany) in
  let pair_type = TMT.pair r (r, Tany) (r, Tany) in
  let tclass_is_falsy_when_empty env ty =
    Env.can_subtype env ty simple_xml_el ||
    Env.can_subtype env ty container_type && not (Env.can_subtype env ty pair_type)
  in
  let trv = TMT.traversable r (r, Tany) in
  let is_traversable env ty = Env.can_subtype env ty trv in
  tclass_is_falsy_when_empty, is_traversable

(** Return the {!truthiness} of {ty}. Only possibly-falsy types are suitable
    scrutinees in a truthiness test--it is reasonable to test the truthiness of
    nullable types and container types which are falsy when empty. Other types
    (e.g. user-defined objects) are always truthy, so testing their truthiness
    indicates a logic error. *)
let rec truthiness env ty =
  let env, ty = Env.fold_unresolved env ty in
  let env, ty = Env.expand_type env ty in
  match snd ty with
  | Tany | Terr | Tdynamic -> Unknown

  | Tnonnull
  | Tabstract (AKenum _, _)
  | Tarraykind _
  | Toption _ -> Possibly_falsy

  | Tclass ((_, cid), _, _) ->
    if cid = SN.Classes.cStringish then Possibly_falsy else
    if tclass_is_falsy_when_empty env ty then Possibly_falsy else
    if not (is_traversable env ty) then Always_truthy else
    (* Classes which implement Traversable but not Container will always be
       truthy when empty. If this Tclass is instead an interface type like
       KeyedTraversable, the value may or may not be truthy when empty. *)
    begin match Typing_lazy_heap.get_class (Env.get_tcopt env) cid with
    | Some {tc_kind = Cnormal | Cabstract; _} -> Always_truthy
    | Some {tc_kind = Cinterface | Cenum; _} -> Possibly_falsy
    | Some {tc_kind = Ctrait; _} | None -> Unknown
    end

  | Tprim Tresource -> Always_truthy
  | Tprim Tnull -> Always_falsy
  | Tprim Tvoid -> Always_falsy
  | Tprim Tnoreturn -> Unknown
  | Tprim (Tint | Tbool | Tfloat | Tstring | Tnum | Tarraykey) -> Possibly_falsy

  | Tunresolved tyl ->
    begin match List.map tyl (truthiness env) with
    | [] -> Unknown
    | hd :: tl -> List.fold tl ~init:hd ~f:fold_truthiness
    end
  | Tabstract _ ->
    let env, tyl = Env.get_concrete_supertypes env ty in
    begin match List.map tyl (truthiness env) with
    | [] -> Unknown
    | hd :: tl -> List.fold tl ~init:hd ~f:fold_truthiness
    end

  | Tshape (FieldsFullyKnown, fields)
    when ShapeMap.cardinal fields = 0 -> Always_falsy
  | Tshape (_, fields) ->
    let has_non_optional_fields =
      ShapeMap.fold (fun _ {sft_optional=opt; _} -> (||) (not opt)) fields false
    in
    if has_non_optional_fields
    then Always_truthy
    else Possibly_falsy

  | Ttuple [] -> Always_falsy
  | Tobject | Tfun _ | Ttuple _ | Tanon _ -> Always_truthy
  | Tvar _ ->
    if TypecheckerOptions.new_inference (Env.get_tcopt env)
    then Unknown
    else failwith "expand_type failed"

(** When a type represented by one of these variants is used in a truthiness
    test, it indicates a potential logic error, since the truthiness of some
    values in the type may be surprising. *)
type sketchy_type_kind =
  | Traversable_interface of Env.t * Tast.ty
  (** Interface types which implement Traversable but not Container may be
      always truthy, even when empty. *)

let rec find_sketchy_types env acc ty =
  let env, ty = Env.fold_unresolved env ty in
  let env, ty = Env.expand_type env ty in
  match snd ty with
  | Toption ty -> find_sketchy_types env acc ty

  | Tclass ((_, cid), _, _) ->
    if tclass_is_falsy_when_empty env ty || not (is_traversable env ty)
    then acc
    else begin
      match Typing_lazy_heap.get_class (Env.get_tcopt env) cid with
      | Some {tc_kind = Cinterface; _} -> Traversable_interface (env, ty) :: acc
      | Some {tc_kind = Cnormal | Cabstract | Ctrait | Cenum; _} | None -> acc
    end

  | Tunresolved tyl ->
    List.fold tyl ~init:acc ~f:(find_sketchy_types env)
  | Tabstract _ ->
    let env, tyl = Env.get_concrete_supertypes env ty in
    List.fold tyl ~init:acc ~f:(find_sketchy_types env)

  | Tany | Tnonnull | Tdynamic | Terr | Tobject | Tprim _ | Tfun _ | Ttuple _
  | Tshape _ | Tvar _ | Tanon _ | Tarraykind _ -> acc

let find_sketchy_types env ty = find_sketchy_types env [] ty
