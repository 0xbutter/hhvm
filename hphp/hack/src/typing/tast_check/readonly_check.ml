(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Aast
module Env = Tast_env
module Cls = Decl_provider.Class
module SN = Naming_special_names

type rty =
  | Readonly
  | Mut [@deriving show]

type ctx = {
  lenv: rty SMap.t;
  (* whether the method/function returns readonly, and a Pos.t for error messages *)
  ret_ty: rty option;
  (* Whether $this is readonly and a Pos.t for error messages *)
  this_ty: rty option;
}

let empty_ctx = { lenv = SMap.empty; ret_ty = None; this_ty = None }

let readonly_kind_to_rty = function
  | Some Ast_defs.Readonly -> Readonly
  | _ -> Mut

let lenv_from_params (params : Tast.fun_param list) : rty SMap.t =
  let result = SMap.empty in
  List.fold_left
    params
    ~f:(fun acc p ->
      SMap.add p.param_name (readonly_kind_to_rty p.param_readonly) acc)
    ~init:result

let get_local lenv id =
  match SMap.find_opt id lenv with
  | Some r -> r
  | None -> Mut

let check =
  object (self)
    inherit Tast_visitor.iter as super

    val mutable ctx : ctx = empty_ctx

    method ty_expr (e : Tast.expr) : rty =
      match e with
      | (_, ReadonlyExpr _) -> Readonly
      | (_, This) -> Option.value ctx.this_ty ~default:Mut
      | (_, Lvar (_, lid)) ->
        let varname = Local_id.to_string lid in
        get_local ctx.lenv varname
      (* If you have a bunch of property accesses in a row, i.e. $x->foo->bar->baz,
        ty_expr will take linear time, and the full check may take O(n^2) time
        if we recurse on expressions in the visitor. We expect this to generally
        be quite small, though. *)
      | (_, Obj_get (e1, _, _, _)) -> self#ty_expr e1
      | _ -> Mut

    method assign lval rval =
      match lval with
      | (_, Obj_get (e1, _, _, _)) ->
        begin
          match self#ty_expr e1 with
          | Readonly -> Errors.readonly_modified (Tast.get_position e1)
          | Mut -> ()
        end
      | (_, Lvar (_, lid)) ->
        let var_ro_opt = SMap.find_opt (Local_id.to_string lid) ctx.lenv in
        begin
          match (var_ro_opt, self#ty_expr rval) with
          | (Some Readonly, Mut) ->
            Errors.var_readonly_mismatch
              (Tast.get_position lval)
              "readonly"
              (Tast.get_position rval)
              "mutable"
          | (Some Mut, Readonly) ->
            Errors.var_readonly_mismatch
              (Tast.get_position lval)
              "mutable"
              (Tast.get_position rval)
              "readonly"
          | (None, r) ->
            (* If it's a new assignment, add to the lenv *)
            let new_lenv = SMap.add (Local_id.to_string lid) r ctx.lenv in
            ctx <- { ctx with lenv = new_lenv }
          | (Some Mut, Mut) -> ()
          | (Some Readonly, Readonly) -> ()
        end
      (* TODO: awaitables *)
      (* TODO: vecs, collections, array accesses *)
      | _ -> ()

    method! on_method_ env m =
      let this_ty =
        if m.m_readonly_this then
          Some Readonly
        else
          Some Mut
      in
      let new_ctx =
        {
          this_ty;
          ret_ty = Some (readonly_kind_to_rty m.m_readonly_ret);
          lenv = lenv_from_params m.m_params;
        }
      in
      ctx <- new_ctx;
      super#on_method_ env m

    method! on_fun_ env f =
      (* TODO: handle lambdas, which capture values. Const lambdas should make every value captured readonly. *)
      let ret_ty = Some (readonly_kind_to_rty f.f_readonly_ret) in
      let new_ctx =
        { this_ty = None; ret_ty; lenv = lenv_from_params f.f_params }
      in
      ctx <- new_ctx;
      super#on_fun_ env f

    method! on_expr env e =
      (* TODO: Handle RO_EXPLICIT_CAST (funs and props returning readonly require explicit cast) *)
      (* TODO: Handle RO_METHOD (method calls on readonly things) *)
      match e with
      | (_, Binop (Ast_defs.Eq _, lval, rval)) ->
        self#assign lval rval;
        super#on_expr env e
      | _ -> super#on_expr env e
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_method_ env m = check#on_method_ env m

    method! at_fun_ env f = check#on_fun_ env f
  end
