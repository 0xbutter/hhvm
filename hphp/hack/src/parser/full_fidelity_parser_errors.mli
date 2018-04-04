(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module WithSyntax : functor (Syntax : Syntax_sig.Syntax_S) -> sig
module WithSmartConstructors : functor (SmartConstructors : SmartConstructors.SmartConstructors_S
  with type r = Syntax.t
  with module Token = Syntax.Token
) -> sig
  type error_level = Minimum | Typical | Maximum

  type hhvm_compat_mode = NoCompat | HHVMCompat | SystemLibCompat

  type env
  val make_env :
    (* Optional parts *)
       ?level:error_level
    -> ?hhvm_compat_mode:hhvm_compat_mode
    -> ?enable_hh_syntax:bool
    -> ?disallow_elvis_space:bool
    (* Required parts *)
    -> Full_fidelity_syntax_tree.WithSyntax(Syntax).WithSmartConstructors(SmartConstructors).t
    -> env

  val parse_errors : env -> Full_fidelity_syntax_error.t list

end (* WithSmartConstructors *)

include module type of WithSmartConstructors(SyntaxSmartConstructors.WithSyntax(Syntax))

end (* WithSyntax *)
