(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Instruction_sequence

let from_ast_no_memoization : Ast.fun_ -> Hhas_function.t =
  fun ast_fun ->
  let function_name = Litstr.to_string @@ snd ast_fun.Ast.f_name in
  let body_instrs,
      function_decl_vars,
      function_params,
      function_return_type =
    Emit_body.from_ast
      ~self:None
      ast_fun.Ast.f_tparams
      ast_fun.Ast.f_params
      ast_fun.Ast.f_ret
      ast_fun.Ast.f_body
  in
  let function_body = instr_seq_to_list body_instrs in
  let function_attributes =
    Emit_attribute.from_asts ast_fun.Ast.f_user_attributes in
  let function_is_async =
    ast_fun.Ast.f_fun_kind = Ast_defs.FAsync
    || ast_fun.Ast.f_fun_kind = Ast_defs.FAsyncGenerator
  in
  Hhas_function.make
    function_attributes
    function_name
    function_params
    function_return_type
    function_body
    function_decl_vars
    function_is_async

let from_ast : Ast.fun_ -> Hhas_function.t list =
  fun ast_fun ->
  let compiled = from_ast_no_memoization ast_fun in
  if Hhas_attribute.is_memoized (Hhas_function.attributes compiled) then
    let (renamed, memoized) = Generate_memoized.memoize_function compiled in
    [ renamed; memoized ]
  else
    [ compiled ]

let from_asts ast_functions =
  Core.List.bind ast_functions from_ast
