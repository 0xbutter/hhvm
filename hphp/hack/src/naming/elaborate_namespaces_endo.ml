(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Core_kernel
module NS = Namespaces
module SN = Naming_special_names

type env = {
  namespace: Namespace_env.env;
  let_locals: SSet.t;
  in_ppl: bool;
}

let elaborate_type_name env id =
  NS.elaborate_id env.namespace NS.ElaborateClass id

let namespace_elaborater =
  object (self)
    inherit [_] Aast.endo as super

    method on_'ex _ ex = ex

    method on_'fb _ fb = fb

    method on_'en _ en = en

    method on_'hi _ hi = hi

    (* Namespaces were already precomputed by ElaborateDefs
     * The following functions just set the namespace env correctly
     *)
    method! on_class_ env c =
      let in_ppl =
        Attributes.mem
          SN.UserAttributes.uaProbabilisticModel
          c.c_user_attributes
      in
      let env = { env with namespace = c.c_namespace; in_ppl } in
      super#on_class_ env c

    method! on_typedef env td =
      let env = { env with namespace = td.t_namespace } in
      super#on_typedef env td

    method! on_fun_def env f =
      let env = { env with namespace = f.f_namespace } in
      super#on_fun_def env f

    method! on_gconst env gc =
      let env = { env with namespace = gc.cst_namespace } in
      super#on_gconst env gc

    method! on_file_attribute env fa =
      let env = { env with namespace = fa.fa_namespace } in
      super#on_file_attribute env fa

    method! on_func_body env fb =
      let env =
        match fb.fb_annotation with
        | Nast.Named
        | Nast.NamedWithUnsafeBlocks ->
          env
        | Nast.Unnamed nsenv -> { env with namespace = nsenv }
      in
      super#on_func_body env fb

    method! on_record_def env rd =
      let env = { env with namespace = rd.rd_namespace } in
      super#on_record_def env rd

    (* Sets let local env correctly *)
    method on_block_helper env b =
      let aux (env, stmts) stmt =
        match stmt with
        | (ann, Let (((_, lid) as x), h, e)) ->
          let new_env =
            {
              env with
              let_locals = SSet.add (Local_id.get_name lid) env.let_locals;
            }
          in
          let new_stmt_ =
            Let (x, Option.map h (super#on_hint env), super#on_expr env e)
          in
          (new_env, (ann, new_stmt_) :: stmts)
        | _ -> (env, super#on_stmt env stmt :: stmts)
      in
      let (env, rev_stmts) = List.fold b ~f:aux ~init:(env, []) in
      (env, List.rev rev_stmts)

    method! on_block env b =
      let (_, stmts) = self#on_block_helper env b in
      stmts

    method! on_catch env (x1, ((_, lid2) as x2), b) =
      (* If the variable does not begin with $, it is an immutable binding *)
      let name2 = Local_id.get_name lid2 in
      let let_locals =
        if name2 <> "" && name2.[0] <> '$' then
          SSet.add name2 env.let_locals
        else
          env.let_locals
      in
      let b = self#on_block { env with let_locals } b in
      (x1, x2, b)

    method! on_stmt_ env stmt =
      match stmt with
      | Foreach (e, ae, b) ->
        let add_local env e =
          match snd e with
          | Id x -> { env with let_locals = SSet.add (snd x) env.let_locals }
          | _ -> env
        in
        let new_env =
          match ae with
          | As_v ve
          | Await_as_v (_, ve) ->
            add_local env ve
          | As_kv (ke, ve)
          | Await_as_kv (_, ke, ve) ->
            let env = add_local env ke in
            add_local env ve
        in
        let e = self#on_expr env e in
        let b = self#on_block new_env b in
        Foreach (e, ae, b)
      | For (e1, e2, e3, b) ->
        let e1 = self#on_expr env e1 in
        let e2 = self#on_expr env e2 in
        let (env, b) = self#on_block_helper env b in
        let e3 = self#on_expr env e3 in
        For (e1, e2, e3, b)
      | Do (b, e) ->
        let (env, b) = self#on_block_helper env b in
        let e = self#on_expr env e in
        Do (b, e)
      (* invariant is handled differently in naming depending whether it is in
       * the statement position or expression position.
       *)
      | Expr (p, Call (p2, (p3, Id (p4, fn)), targs, el, uel))
        when fn = SN.SpecialFunctions.invariant ->
        Expr
          ( p,
            Call
              ( p2,
                (p3, Id (p4, fn)),
                List.map targs ~f:(self#on_targ env),
                List.map el ~f:(self#on_expr env),
                List.map uel ~f:(self#on_expr env) ) )
      | _ -> super#on_stmt_ env stmt

    (* Lambda environments *)
    method! on_Lfun env e =
      let env = { env with in_ppl = false } in
      super#on_Lfun env e

    method! on_Efun env e =
      let env = { env with in_ppl = false } in
      super#on_Efun env e

    (* The function that actually rewrites names *)
    method! on_expr_ env expr =
      match expr with
      | Call (ct, (p, Id (p2, cn)), targs, [(p3, String fn)], uargs)
        when cn = SN.SpecialFunctions.fun_ ->
        (* Functions referenced by fun() are always fully-qualified *)
        let fn = Utils.add_ns fn in
        Call (ct, (p, Id (p2, cn)), targs, [(p3, String fn)], uargs)
      | Call (ct, (p, Id ((_, cn) as id)), tal, el, uel)
        when cn = SN.SpecialFunctions.invariant ->
        (* invariant is handled differently in naming depending whether it is in
         * the statement position or expression position.
         *)
        let new_id = NS.elaborate_id env.namespace NS.ElaborateFun id in
        Call
          ( ct,
            (p, Id new_id),
            List.map tal ~f:(self#on_targ env),
            List.map el ~f:(self#on_expr env),
            List.map uel ~f:(self#on_expr env) )
      | Call (ct, (p, Id (p2, cn)), targs, el, uargs)
        when SN.SpecialFunctions.is_special_function cn
             || (SN.PPLFunctions.is_reserved cn && env.in_ppl) ->
        Call
          ( self#on_call_type env ct,
            (p, Id (p2, cn)),
            List.map targs ~f:(self#on_targ env),
            List.map el ~f:(self#on_expr env),
            List.map uargs ~f:(self#on_expr env) )
      | Call (ct, (p, Aast.Id id), tal, el, uel) ->
        let new_id =
          if SSet.mem (snd id) env.let_locals then
            id
          else
            NS.elaborate_id env.namespace NS.ElaborateFun id
        in
        Call
          ( ct,
            (p, Id new_id),
            List.map tal ~f:(self#on_targ env),
            List.map el ~f:(self#on_expr env),
            List.map uel ~f:(self#on_expr env) )
      | Obj_get (e1, (p, Id x), null_safe) ->
        Obj_get (self#on_expr env e1, (p, Id x), null_safe)
      | Id ((_, name) as sid) ->
        if SSet.mem name env.let_locals then
          expr
        else
          Id (NS.elaborate_id env.namespace NS.ElaborateConst sid)
      | Class_get ((_, CIexpr (_, Id _)), CGstring _) -> expr
      | Class_const ((_, Aast.CIexpr (_, Aast.Id _)), _) -> expr
      | PU_identifier ((_, CIexpr (_, Id _)), _, _) -> expr
      | New ((p1, CIexpr (p2, Id x)), tal, el, uel, ex) ->
        New
          ( (p1, CIexpr (p2, Id x)),
            List.map tal ~f:(self#on_targ env),
            List.map el ~f:(self#on_expr env),
            List.map uel ~f:(self#on_expr env),
            ex )
      | Record (((_, CIexpr (_, Id _)) as n), is_array, l) ->
        let l =
          List.map l ~f:(fun (e1, e2) ->
              (self#on_expr env e1, self#on_expr env e2))
        in
        Record (n, is_array, l)
      | _ -> super#on_expr_ env expr

    method! on_user_attribute env ua =
      let ua_name =
        if SN.UserAttributes.is_reserved (snd ua.ua_name) then
          ua.ua_name
        else
          elaborate_type_name env ua.ua_name
      in
      { ua with ua_name }

    method! on_program (env : env) (p : Nast.program) =
      let aux (env, defs) def =
        match def with
        | SetNamespaceEnv nsenv ->
          let env = { env with namespace = nsenv } in
          (env, def :: defs)
        | _ -> (env, super#on_def env def :: defs)
      in
      let (_, rev_defs) = List.fold p ~f:aux ~init:(env, []) in
      List.rev rev_defs
  end

(* Toplevel environment *)
let make_env namespace = { namespace; let_locals = SSet.empty; in_ppl = false }
