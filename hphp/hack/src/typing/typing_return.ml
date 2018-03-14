(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* Typing code concerned with return types. *)

open Typing_defs
open Typing_env_return_info
open Hh_core

module Env = Typing_env

(* For async functions, strip Awaitable<_> from the return type *)
let strip_awaitable fun_kind env ty =
  if fun_kind <> Ast.FAsync then ty
  else
  match Env.expand_type env ty with
  | _env, (_, Tclass ((_, class_name), [ty]))
    when class_name = Naming_special_names.Classes.cAwaitable ->
    ty
    (* In non-strict code we might find Awaitable without type arguments. Assume Tany *)
  | _env, (_, Tclass ((_, class_name), []))
    when class_name = Naming_special_names.Classes.cAwaitable ->
    (Reason.Rnone, Tany)
  | _ ->
    ty

let enforce_return_not_disposable fun_kind env ty =
  match Typing_disposable.is_disposable_type env (strip_awaitable fun_kind env ty) with
  | Some class_name ->
    Errors.invalid_disposable_return_hint (Reason.to_pos (fst ty))
      (Utils.strip_ns class_name)
  | None ->
    ()

let has_attribute attr l =
  List.exists l (fun { Nast.ua_name; _ } -> attr = snd ua_name)

let has_return_disposable_attribute attrs =
  has_attribute SN.UserAttributes.uaReturnDisposable attrs

let has_mutable_return_attribute attrs =
  has_attribute SN.UserAttributes.uaMutableReturn attrs

let make_info fun_kind attributes env ~is_explicit ty =
  let return_disposable = has_return_disposable_attribute attributes in
  let return_mutable = has_mutable_return_attribute attributes in
  if not return_disposable
  then enforce_return_not_disposable fun_kind env ty;
  {
    return_type = ty;
    return_disposable;
    return_mutable;
    return_explicit = is_explicit;
  }

(* For async functions, wrap Awaitable<_> around the return type *)
let wrap_awaitable env p rty =
  match Env.get_fn_kind env with
    | Ast.FCoroutine
    | Ast.FSync ->
      rty
    | Ast.FGenerator
      (* Is an error, but caught in NastCheck. *)
    | Ast.FAsyncGenerator ->
      (Reason.Rnone, Typing_utils.terr env)
    | Ast.FAsync ->
      (Reason.Rwitness p), Tclass ((p, SN.Classes.cAwaitable), [rty])

let force_awaitable env p ty =
  let fun_kind = Env.get_fn_kind env in
  match Env.expand_type env ty with
  | env, (_, Tclass ((_, class_name), _))
    when fun_kind = Ast.FAsync && class_name = Naming_special_names.Classes.cAwaitable ->
    env, ty
  | env, (_, Tany) when fun_kind = Ast.FAsync ->
    env, wrap_awaitable env p ty
  | _ when fun_kind = Ast.FAsync ->
    let env, underlying_ty = Env.fresh_unresolved_type env in
    let wrapped_ty = wrap_awaitable env p underlying_ty in
    let env = Typing_subtype.sub_type env wrapped_ty ty in
    env, wrapped_ty
  | _ ->
    env, ty

(* If there is no return type annotation on method, assume `void` for the
 * special functions `__destruct` and `__construct`, otherwise Tany *)
let make_default_return env name =
  if snd name = SN.Members.__destruct
  || snd name = SN.Members.__construct
  then (Reason.Rwitness (fst name), Tprim Nast.Tvoid)
  else (Reason.Rwitness (fst name), Typing_utils.tany env)

let suggest_return env p ty =
  let ty = Typing_expand.fully_expand env ty in
  (match Typing_print.suggest ty with
  | "..." -> Errors.expecting_return_type_hint p
  | ty -> Errors.expecting_return_type_hint_suggest p ty
)

let async_suggest_return fkind hint pos =
  let is_async = Ast_defs.FAsync = fkind in
  if is_async then
    let e_func = Errors.expecting_awaitable_return_type_hint in
    match snd hint with
    | Nast.Happly (s, _) ->
        if snd s <> Naming_special_names.Classes.cAwaitable then e_func pos
    | _ -> e_func pos
