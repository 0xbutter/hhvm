(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let visitor = Tast_visitor.iter_with [
]

let program = visitor#go
let def = visitor#go_def
