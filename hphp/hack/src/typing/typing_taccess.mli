(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Typing_defs

(* Expands a type access to underlying type *)
val expand:
  Typing_env.env ->
  Typing_reason.t ->
  taccess_type ->
  Typing_env.env * locl ty

(* Returns (class_name, tconst_name, tconst_reference_position) for each type
 * constant referenced in a type access. *)
val referenced_typeconsts:
  Typing_env.env ->
  expand_env ->
  Typing_reason.t ->
  taccess_type ->
  (string * string * Pos.t) list
