(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SyntaxError = Full_fidelity_syntax_error
module SyntaxTree = Full_fidelity_syntax_tree
module SourceText = Full_fidelity_source_text

let usage = Printf.sprintf "Usage: %s filename\n" Sys.argv.(0)

type t1 = {
  mutable s: string
}

type t2 = {
  f1: t1
}

let parse_and_print filename =
  let file = Relative_path.create Relative_path.Dummy filename in
  let source_text = SourceText.from_file file in
  let syntax_tree = SyntaxTree.make source_text in

  let errors = SyntaxTree.errors syntax_tree in
  let printer err = Printf.printf "%s\n" (
    SyntaxError.to_positioned_string
      err (SourceText.offset_to_position source_text)
  ) in
  let str = Debug.dump_full_fidelity syntax_tree in
  Printf.printf "%s\n" (SourceText.get_text source_text);
  Printf.printf "%s\n" str;
  let editable = Full_fidelity_editable_syntax.from_tree syntax_tree in
  let chunks = Hack_format.run editable in
  let init_state = Solve_state.make chunks in
  State_queue.add init_state;
  let result = Line_splitter.solve () in
  Printf.printf "%s\n" (Solve_state.__debug result);
  Printf.printf "Formatting result:\n%s\n" (State_printer.print_state result);
  ()


let () =
  Arg.parse [] parse_and_print usage;
