(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core
open Instruction_sequence

let from_ast : Ast.class_ -> Ast.method_ -> Hhas_method.t option =
  fun ast_class ast_method ->
  let method_name = Litstr.to_string @@ snd ast_method.Ast.m_name in
  let method_is_abstract = List.mem ast_method.Ast.m_kind Ast.Abstract in
  let method_is_final = List.mem ast_method.Ast.m_kind Ast.Final in
  let method_is_private = List.mem ast_method.Ast.m_kind Ast.Private in
  let method_is_protected = List.mem ast_method.Ast.m_kind Ast.Protected in
  let method_is_public = List.mem ast_method.Ast.m_kind Ast.Public in
  let method_is_static = List.mem ast_method.Ast.m_kind Ast.Static in
  let method_attributes =
    Emit_attribute.from_asts ast_method.Ast.m_user_attributes in
  match ast_method.Ast.m_body with
  | b ->
    let tparams = ast_class.Ast.c_tparams @ ast_method.Ast.m_tparams in
    let body_instrs, method_params, method_return_type = Emit_body.from_ast
      tparams ast_method.Ast.m_params ast_method.Ast.m_ret b in
    let method_body = instr_seq_to_list body_instrs in
    let m = Hhas_method.make
      method_attributes
      method_is_protected
      method_is_public
      method_is_private
      method_is_static
      method_is_final
      method_is_abstract
      method_name
      method_params
      method_return_type
      method_body in
    Some m

let from_asts ast_class ast_methods =
  List.filter_map ast_methods (from_ast ast_class)
