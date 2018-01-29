(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Syntax = Full_fidelity_positioned_syntax
module SyntaxParser = Full_fidelity_parser.WithSyntax(Syntax)
module SC = Full_fidelity_syntax_smart_constructors.WithSyntax(Syntax)
include SyntaxParser.WithSmartConstructors(SC)
