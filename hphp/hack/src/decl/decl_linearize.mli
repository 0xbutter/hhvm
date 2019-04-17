(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Decl_defs

val get_linearization : string -> linearization

val push_local_changes : unit -> unit
val pop_local_changes : unit -> unit

val remove_batch : SSet.t -> unit
