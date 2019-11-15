(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* cf: typing_env_types_sig.mli - These files should be the same *)
open Hh_prelude
open Typing_defs
module TPEnv = Type_parameter_env
module ITySet = Internal_type_set

type locl_ty = Typing_defs.locl_ty

[@@@warning "-32"]

let show_local_id_set_t _ = "<local_id_set_t>"

let pp_local_id_set_t _ _ = Printf.printf "%s\n" "<local_id_set_t>"

type local_id_set_t = Local_id.Set.t

let show_local_env _ = "<local_env>"

let pp_local_env _ _ = Printf.printf "%s\n" "<local_env>"

(* Local environment includes types of locals and bounds on type parameters. *)
type local_env = {
  per_cont_env: Typing_per_cont_env.t;
  local_mutability: Typing_mutability_env.mutability_env;
  (* Whether current environment is reactive *)
  local_reactive: reactivity;
  (* Local variables that were assigned in a `using` clause *)
  local_using_vars: local_id_set_t;
}

let show_env _ = "<env>"

let pp_env _ _ = Printf.printf "%s\n" "<env>"

let show_genv _ = "<genv>"

let pp_genv _ _ = Printf.printf "%s\n" "<genv>"

let show_anon _ = "<anon>"

let pp_anon _ _ = Printf.printf "%s\n" "<anon>"

let show_tfun _ = "<tfun>"

let pp_tfun _ _ = Printf.printf "%s\n" "<tfun>"

[@@@warning "+32"]

type tyvar_info_ = {
  (* Where was the type variable introduced? (e.g. generic method invocation,
   * new object construction)
   *)
  tyvar_pos: Pos.t;
  (* Set to true if a call to expand_type_and_solve on this variable cannot resolve
   *)
  eager_solve_fail: bool;
  (* Does this type variable appear covariantly in the type of the expression?
   *)
  appears_covariantly: bool;
  (* Does this type variable appear contravariantly in the type of the expression?
   * If it appears in an invariant position then both will be true; if it doesn't
   * appear at all then both will be false
   *)
  appears_contravariantly: bool;
  lower_bounds: ITySet.t;
  upper_bounds: ITySet.t;
  (* Map associating a type to each type constant id of this variable.
  Whenever we localize "T1::T" in a constraint, we add a fresh type variable
  indexed by "T" in the type_constants of the type variable representing T1.
  This allows to properly check constraints on "T1::T". *)
  type_constants:
    ( Aast.sid (* id of the type constant "T", containing its position. *)
    * locl_ty )
    SMap.t;
}
[@@deriving eq]

(* For global inference we are distinguishing global tyvars and local tyvars.
Global tyvars are tyvars which have to remain unsolved until all of the files
are done typechecking. Thus, global tyvars are going possess their own constraint
graph which is global_tvenv. Local tyvars have the same behavior as a "normal"
tyvar.

Concerning the implementation, the tvenv can now hold either "LocalTyvar tyvar_info"
if the tyvar is local or a "GlobalTyvar" atom if the tyvar is global and thus
can be found in the globalenv.
*)
type tyvar_info =
  | LocalTyvar of tyvar_info_
  | GlobalTyvar
[@@deriving eq]

type tvenv = tyvar_info IMap.t

type global_tvenv = tyvar_info_ IMap.t

type env = {
  (* position of the function/method being checked *)
  function_pos: Pos.t;
  (* Mapping of type variables to types. *)
  tenv: locl_ty IMap.t;
  (* Mapping of type variables to other type variables *)
  subst: int IMap.t;
  tyvar_occurrences: ISet.t IMap.t;
      (** A map to track where each type variable occurs,
          more precisely in the type of which other type variables.
          E.g. if #1 is bound to (#2 | int), then this map contains the entry
            #2 -> { #1 }
          This is based on shallow binding, i.e. in the example above, if #2
          is mapped to #3, then tyvar_occurrences would be:
            #2 -> { #1 }
            #3 -> { #2 }
          but we would record that #3 occurs in #1.
          When a type variable v gets solved or the type bound to it gets simplified, 
          we simplify the unions and intersections of the types bound to the 
          type variables associated to v in this map.
          So in our example, if #2 gets solved to int,
          we simplify #1 to (int | int) = int. 
          There are only entries for variables that are unsolved or contain 
          other unsolved type variables. Variables that are solved and contain 
          no other unsolved type variables get removed from this map. *)
  tyvars_in_tyvar: ISet.t IMap.t;
      (** Mapping of type variables to the type variables contained in their 
          types which are either unsolved or themselves contain unsolved type
          variables.
          This is the dual of tyvar_occurrences. *)
  fresh_typarams: SSet.t;
  lenv: local_env;
  genv: genv;
  decl_env: Decl_env.env;
  in_loop: bool;
  in_try: bool;
  in_case: bool;
  inside_constructor: bool;
  inside_ppl_class: bool;
  (* A set of constraints that are global to a given method *)
  global_tpenv: TPEnv.t;
  subtype_prop: Typing_logic.subtype_prop;
  log_levels: int SMap.t;
  tvenv: tvenv;
  global_tvenv: global_tvenv;
  tyvars_stack: (Pos.t * Ident.t list) list;
  allow_wildcards: bool;
  big_envs: (Pos.t * env) list ref;
  pessimize: bool;
}

and genv = {
  tcopt: TypecheckerOptions.t;
  return: Typing_env_return_info.t;
  (* For each function parameter, its type and calling convention. *)
  params: (locl_ty * param_mode) Local_id.Map.t;
  (* condition types associated with parameters.
     For every mayberx parameter that has condition type we create
     fresh type parameter (see: make_local_param_ty) and store mapping
     fresh type name -> condition type in env so it can be retrieved later *)
  condition_types: decl_ty SMap.t;
  (* Identifier and type of the parent class if it exists *)
  parent: (string * decl_ty) option;
  (* Identifier and type (instatiated at its generic parameters) of
     the enclosing class if there is one *)
  self: (string * locl_ty) option;
  static: bool;
  fun_kind: Ast_defs.fun_kind;
  val_kind: Typing_defs.val_kind;
  fun_mutable: param_mutability option;
  anons: anon IMap.t;
  file: Relative_path.t;
}

(* A type-checker for an anonymous function
 * Parameters are
 * - the environment
 * - types of the parameters under which the body should be checked
 * - the arity of the function
 * - the expected return type of the body (optional)
 *)
and anon_log = locl_ty list * locl_ty list

and anon = {
  rx: reactivity;
  is_coroutine: Aast.is_coroutine;
  counter: anon_log ref;
  pos: Pos.t;
  typecheck:
    ?el:Nast.expr list ->
    ?ret_ty:locl_ty ->
    env ->
    locl_fun_params ->
    locl_fun_arity ->
    env * Tast.expr * locl_ty;
}

let env_reactivity env = env.lenv.local_reactive
