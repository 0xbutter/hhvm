(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open ServerInitTypes

val init :
  load_mini_approach: load_mini_approach option ->
  ServerEnv.genv ->
  lazy_level ->
  ServerEnv.env ->
  Path.t ->
  (ServerEnv.env * float) * (loaded_info * files_changed_while_parsing, error) result
