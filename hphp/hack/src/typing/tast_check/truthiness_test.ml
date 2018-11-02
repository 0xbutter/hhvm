(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Ast_defs
open Tast

module Env = Tast_env
module SN = Naming_special_names

let rec truthiness_test env ((p, ty), e) =
  match e with
  (* An Expr_list, e.g. a `for` loop condition, will be annotated with a Ttuple
     type, but we are only interested in the type of the last expression, which
     is the one that will be used for the condition. *)
  | Expr_list [] -> ()
  | Expr_list el -> truthiness_test env (List.last_exn el)
  | _ ->
    let open Tast_utils in
    List.iter (find_sketchy_types env ty) begin function
      | Traversable_interface (env, ty) ->
        Errors.sketchy_truthiness_test p (Env.print_ty env ty) `Traversable
    end;
    match truthiness env ty with
    | Always_truthy -> Errors.invalid_truthiness_test p (Env.print_ty env ty)
    | Possibly_falsy | Always_falsy | Unknown -> ()

let handler = object
  inherit Tast_visitor.handler_base

  method! minimum_forward_compat_level = 2018_09_18

  method! at_expr env x =
    if Env.is_strict env then
    match snd x with
    | Unop (Unot, e)
    | Eif (e, _, _)
    | Assert (AE_assert e) -> truthiness_test env e
    | Binop (Ampamp, e1, e2)
    | Binop (Barbar, e1, e2) ->
      truthiness_test env e1;
      truthiness_test env e2;
    | _ -> ()

  method! at_stmt env x =
    if Env.is_strict env then
    match x with
    | If (e, _, _)
    | Do (_, e)
    | While (e, _)
    | For (_, e, _, _) -> truthiness_test env e
    | _ -> ()
end
