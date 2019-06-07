(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* An alias for the errors type that we marshal to and unmarshal from the saved state *)
type saved_state_errors = (Errors.phase * Relative_path.Set.t) list

type save_state_result = {
  naming_table_rows_changed : int;
  dep_table_edges_added : int;
}
