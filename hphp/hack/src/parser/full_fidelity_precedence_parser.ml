(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
module type Lexer_S = Full_fidelity_lexer_sig.WithToken(Syntax.Token).Lexer_S
module Context = Full_fidelity_parser_context.WithToken(Syntax.Token)
module type SC_S = SmartConstructors.SmartConstructors_S

module WithLexer(Lexer : Lexer_S) = struct
  module Lexer = Lexer

  module WithSmartConstructors (SC : SC_S with type token = Syntax.Token.t)
  = struct
    module SC = SC

type t = {
  lexer : Lexer.t;
  errors : Full_fidelity_syntax_error.t list;
  context: Context.t;
  precedence : int;
  env : Full_fidelity_parser_env.t;
  sc_state : SC.t;
}

  let sc_call parser f =
    let (sc_state, result) = f parser.sc_state in
    ({parser with sc_state}, result)

let make env lexer errors context sc_state =
  { lexer; errors; context; precedence = 0; env; sc_state }

let errors parser =
  parser.errors @ (Lexer.errors parser.lexer)

let env parser =
  parser.env

let with_errors parser errors =
  { parser with errors }

let with_lexer parser lexer =
  { parser with lexer }

let lexer parser =
  parser.lexer

let context parser =
  parser.context

let with_context parser context =
  { parser with context }

let skipped_tokens parser =
  Context.skipped_tokens parser.context

let with_skipped_tokens parser skipped_tokens =
  let new_context = Context.with_skipped_tokens
    parser.context skipped_tokens in
  with_context parser new_context

let clear_skipped_tokens parser =
  with_skipped_tokens parser []

(** Wrapper functions for interfacing with parser context **)

let expect parser token_kind_list =
  let new_context = Context.expect parser.context token_kind_list in
  with_context parser new_context

let expect_in_new_scope parser token_kind_list =
  let new_context = Context.expect_in_new_scope
    parser.context token_kind_list in
  with_context parser new_context

let expects parser token_kind =
  Context.expects parser.context token_kind

let expects_here parser token_kind =
  Context.expects_here parser.context token_kind

let pop_scope parser token_kind_list =
  let new_context = Context.pop_scope parser.context token_kind_list in
  with_context parser new_context

let print_expected parser =
  Context.print_expected parser.context

(** Precedence functions **)

let with_precedence parser precedence =
  { parser with precedence }

let with_numeric_precedence parser new_precedence parse_function =
  let old_precedence = parser.precedence in
  let parser = with_precedence parser new_precedence in
  let (parser, result) = parse_function parser in
  let parser = with_precedence parser old_precedence in
  (parser, result)

let with_operator_precedence parser operator parse_function =
  let new_precedence = Full_fidelity_operator.precedence operator in
  with_numeric_precedence parser new_precedence parse_function

let with_reset_precedence parser parse_function =
  with_numeric_precedence parser 0 parse_function

let next_xhp_element_token ?(no_trailing=false) parser =
  let (lexer, token, text) =
    Lexer.next_xhp_element_token ~no_trailing parser.lexer in
  let parser = { parser with lexer } in
  (parser, token, text)

let next_xhp_body_token parser =
  let (lexer, token) = Lexer.next_xhp_body_token parser.lexer in
  let parser = { parser with lexer } in
  (parser, token)

  include SmartConstructors.ParserWrapper(struct
    type parser_type = t
    module SCI = SC
    let call = sc_call
  end)
end (* WithSmartConstructors *)
end (* WithLexer *)
end (* WithSyntax *)
