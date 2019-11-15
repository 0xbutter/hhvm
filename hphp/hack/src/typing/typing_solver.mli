module Env = Typing_env
open Typing_defs
open Typing_env_types

(* Non-side-effecting test for subtypes.
    result = true implies ty1 <: ty2
    result = false implies NOT ty1 <: ty2 OR we don't know
*)
val is_sub_type : env -> locl_ty -> locl_ty -> bool

val use_bind_to_equal_bound : bool ref

val push_option_out : Pos.t -> env -> locl_ty -> env * locl_ty

val non_null : env -> Pos.t -> locl_ty -> env * locl_ty

(* Force solve all remaining unsolved type variables *)
val solve_all_unsolved_tyvars : env -> Errors.typing_error_callback -> env

val expand_type_and_solve :
  env ->
  description_of_expected:string ->
  Pos.t ->
  locl_ty ->
  Errors.typing_error_callback ->
  env * locl_ty

val expand_type_and_solve_eq :
  env -> locl_ty -> Errors.typing_error_callback -> env * locl_ty

val expand_type_and_narrow :
  env ->
  ?default:locl_ty ->
  description_of_expected:string ->
  (env -> locl_ty -> env * locl_ty option) ->
  Pos.t ->
  locl_ty ->
  Errors.typing_error_callback ->
  env * locl_ty

val close_tyvars_and_solve : env -> Errors.typing_error_callback -> env

val expand_bounds_of_global_tyvars : env -> env

val solve_all_unsolved_tyvars_gi :
  env -> (Ident.t -> Errors.typing_error_callback) -> env

val close_tyvars_and_solve_gi :
  env -> (Ident.t -> Errors.typing_error_callback) -> env

val bind : env -> Ident.t -> locl_ty -> env
