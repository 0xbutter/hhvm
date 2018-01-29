(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module type SC_S = Full_fidelity_smart_constructors_sig.SmartConstructors_S

module WithSyntax : functor (Syntax : Syntax_sig.Syntax_S) -> sig
module WithSmartConstructors
: functor (SCI : SC_S with type token = Syntax.Token.t) -> sig
type t
val make : ?env:Full_fidelity_parser_env.t -> Full_fidelity_source_text.t -> t
val from_root :
  Full_fidelity_source_text.t ->
  Syntax.t ->
  Full_fidelity_syntax_error.t list ->
  t
val root : t -> Syntax.t
val text : t -> Full_fidelity_source_text.t
val all_errors : t -> Full_fidelity_syntax_error.t list
val errors : t -> Full_fidelity_syntax_error.t list
val language : t -> string
val mode : t -> string
val is_hack : t -> bool
val is_php : t -> bool
val is_strict : t -> bool
val is_decl : t -> bool
val to_json : ?with_value:bool -> t -> Hh_json.json
end (* WithSmartConstructors *)

(* TODO(T25019784): Remove these declarations. *)
type t
val make : ?env:Full_fidelity_parser_env.t -> Full_fidelity_source_text.t -> t
val from_root :
  Full_fidelity_source_text.t ->
  Syntax.t ->
  Full_fidelity_syntax_error.t list ->
  t
val root : t -> Syntax.t
val text : t -> Full_fidelity_source_text.t
val all_errors : t -> Full_fidelity_syntax_error.t list
val errors : t -> Full_fidelity_syntax_error.t list
val language : t -> string
val mode : t -> string
val is_hack : t -> bool
val is_php : t -> bool
val is_strict : t -> bool
val is_decl : t -> bool
val to_json : ?with_value:bool -> t -> Hh_json.json

end (* WithSyntax *)
