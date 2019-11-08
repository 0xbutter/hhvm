(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Option.Monad_infix
module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_positioned_syntax

(** Returns ((symbol_line, symbol_char), argument_idx) where:
    - symbol_line: the line number of the function symbol
    - symbol_char: the column number of the function symbol
    - argument_idx: index of the function argument that contains the offset.

    For example, given this line:

        25:    $myObject->foo(true, null);
        offset:                    ^

    We would return the following:

        Some ((25, 16), 1)

    Returns None if the given offset is not inside a function call.
*)
let get_positional_info (cst : Syntax.t) (file_offset : int) :
    ((int * int) * int) option =
  Syntax.(
    let parent_tree = parentage cst file_offset in
    (* Search upwards through the parent tree.
     * If we find a function call or constructor, signature help should appear.
     * If we find a lambda first, don't offer help even if we are within a function call! *)
    let within_lambda =
      Option.value
        ~default:false
        (List.find_map parent_tree ~f:(fun syntax ->
             match syntax.syntax with
             | LambdaExpression _ -> Some true
             | ConstructorCall _
             | FunctionCallExpression _ ->
               Some false
             | _ -> None))
    in
    if within_lambda then
      None
    else
      parent_tree
      |> List.find_map ~f:(fun syntax ->
             match syntax.syntax with
             | FunctionCallExpression children ->
               Some
                 ( children.function_call_receiver,
                   children.function_call_argument_list )
             | ConstructorCall children ->
               Some
                 ( children.constructor_call_type,
                   children.constructor_call_argument_list )
             | _ -> None)
      >>= fun (callee_node, argument_list) ->
      trailing_token callee_node
      >>= fun callee_trailing_token ->
      let function_symbol_offset = Token.end_offset callee_trailing_token in
      let pos =
        SourceText.offset_to_position
          callee_trailing_token.Token.source_text
          function_symbol_offset
      in
      let arguments = children argument_list in
      (* Add 1 to counteract the -1 in trailing_end_offset. *)
      let in_args_area =
        leading_start_offset argument_list <= file_offset
        && file_offset <= trailing_end_offset argument_list + 1
      in
      if not in_args_area then
        None
      else
        match arguments with
        | [] -> Some (pos, 0)
        | arguments ->
          arguments
          |> List.mapi ~f:(fun idx elem -> (idx, elem))
          |> List.find_map ~f:(fun (idx, child) ->
                 (* Don't bother range checking if we're in the final argument, since we
          already checked that in in_args_area up above. *)
                 let matches_end =
                   idx = List.length arguments - 1
                   || file_offset < trailing_end_offset child
                 in
                 if matches_end then
                   Some (pos, idx)
                 else
                   None))

let get_occurrence_info
    (env : ServerEnv.env)
    (nast : Nast.program)
    (occurrence : Relative_path.t SymbolOccurrence.t) =
  let (ft_opt, full_occurrence) =
    (* Handle static methods, instance methods, and constructors *)
    match occurrence.SymbolOccurrence.type_ with
    | SymbolOccurrence.Method (classname, methodname) ->
      let classname = Utils.add_ns classname in
      let ft =
        if methodname = "__construct" then
          Decl_provider.get_class_constructor classname
        else
          Option.first_some
            (Decl_provider.get_class_method classname methodname)
            (Decl_provider.get_static_method classname methodname)
      in
      (ft, occurrence)
    | _ ->
      let fun_name =
        ServerEnv.expand_namespace env occurrence.SymbolOccurrence.name
      in
      let ft = Decl_provider.get_fun fun_name in
      let full_occurrence =
        match occurrence.SymbolOccurrence.type_ with
        | SymbolOccurrence.Function ->
          { occurrence with SymbolOccurrence.name = fun_name }
        | _ -> occurrence
      in
      (ft, full_occurrence)
  in
  let def_opt = ServerSymbolDefinition.go (Some nast) full_occurrence in
  match ft_opt with
  | None -> None
  | Some ft -> Some (occurrence, ft, def_opt)

let gather_signature_help env cst nast tast tcopt offset =
  match
    get_positional_info
      (Full_fidelity_ast.PositionedSyntaxTree.root cst)
      offset
  with
  | None -> None
  | Some ((symbol_line, symbol_char), argument_idx) ->
    let results =
      IdentifySymbolService.go ~tast ~line:symbol_line ~column:symbol_char
    in
    let results =
      List.filter results ~f:(fun r ->
          match r.SymbolOccurrence.type_ with
          | SymbolOccurrence.Method _
          | SymbolOccurrence.Function ->
            true
          | _ -> false)
    in
    (match List.hd results with
    | None -> None
    | Some head_result ->
      (match get_occurrence_info env nast head_result with
      | None -> None
      | Some (occurrence, fe, def_opt) ->
        Typing_defs.(
          Lsp.SignatureHelp.(
            let tast_env = Tast_env.empty tcopt in
            let siginfo_label =
              Tast_env.print_ty_with_identity
                tast_env
                (DeclTy fe.fe_type)
                occurrence
                def_opt
            in
            let siginfo_documentation =
              let base_class_name =
                SymbolOccurrence.enclosing_class occurrence
              in
              def_opt
              >>= fun def ->
              let file =
                ServerCommandTypes.FileName
                  (def.SymbolDefinition.pos |> Pos.to_absolute |> Pos.filename)
              in
              ServerDocblockAt.go_comments_for_symbol
                ~def
                ~base_class_name
                ~file
            in
            let param_docs =
              match siginfo_documentation with
              | Some siginfo_documentation ->
                Some (Docblock_parser.get_param_docs siginfo_documentation)
              | None -> None
            in
            let ft_params =
              match fe.fe_type with
              | (_, Tfun ft) -> ft.ft_params
              | _ -> []
            in
            let params =
              List.map ft_params ~f:(fun param ->
                  let parinfo_label =
                    match param.fp_name with
                    | Some s -> s
                    | None -> Tast_env.print_decl_ty tast_env fe.fe_type
                  in
                  let parinfo_documentation =
                    match param_docs with
                    | Some param_docs -> Map.find param_docs parinfo_label
                    | None -> None
                  in
                  { parinfo_label; parinfo_documentation })
            in
            let signature_information =
              { siginfo_label; siginfo_documentation; parameters = params }
            in
            Some
              {
                signatures = [signature_information];
                activeSignature = 0;
                activeParameter = argument_idx;
              }))))

let go
    ~(env : ServerEnv.env)
    ~(file : ServerCommandTypes.file_input)
    ~(line : int)
    ~(column : int) : Lsp.SignatureHelp.result =
  let ServerEnv.{ tcopt; _ } = env in
  let tcopt =
    {
      tcopt with
      GlobalOptions.tco_dynamic_view = ServerDynamicView.dynamic_view_on ();
    }
  in
  let source_text = ServerCommandTypesUtils.source_tree_of_file_input file in
  let relative_path = source_text.SourceText.file_path in
  let parser_env = Full_fidelity_ast.make_env relative_path in
  let (cst, results) =
    Full_fidelity_ast.from_text_with_legacy_and_cst parser_env source_text
  in
  let nast = results.Parser_return.ast in
  let tast = ServerIdeUtils.check_ast tcopt nast in
  let offset = SourceText.position_to_offset source_text (line, column) in
  gather_signature_help env cst nast tast tcopt offset

let go_ctx
    ~(env : ServerEnv.env)
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : Lsp.SignatureHelp.result =
  let cst = Provider_utils.compute_cst ~ctx ~entry in
  let (tast, _) = Provider_utils.compute_tast_and_errors ~ctx ~entry in
  let offset =
    SourceText.position_to_offset
      entry.Provider_context.source_text
      (line, column)
  in
  gather_signature_help
    env
    cst
    entry.Provider_context.ast
    tast
    env.ServerEnv.tcopt
    offset
