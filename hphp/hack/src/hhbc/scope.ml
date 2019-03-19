(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
open Instruction_sequence
open Local

(* Run emit () in a new unnamed local scope. If the emitted code registered any
 * unnamed locals, it will be wrapped in a try/fault that will unset these
 * unnamed locals upon exception. *)
let with_unnamed_locals emit =
  let current_next_local = !next_local in
  let current_temp_local_map = !temp_local_map in
  let result = emit () in
  if current_next_local = !next_local then result else
  let unsets = gather @@
    List.init (!next_local - current_next_local)
      ~f:(fun idx -> instr_unsetl (Unnamed (idx + current_next_local))) in
  next_local := current_next_local;
  temp_local_map := current_temp_local_map;
  let fault_block = gather [ unsets; instr_unwind ] in
  let fault_label = Label.next_fault () in
  instr_try_fault fault_label result fault_block
