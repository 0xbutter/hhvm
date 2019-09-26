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
    parentage cst file_offset
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
  let ft_opt =
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
      ft
    | _ ->
      let fun_name =
        ServerEnv.expand_namespace env occurrence.SymbolOccurrence.name
      in
      let ft = Decl_provider.get_fun fun_name in
      ft
  in
  let def_opt = ServerSymbolDefinition.go (Some nast) occurrence in
  match ft_opt with
  | None -> None
  | Some ft -> Some (occurrence, ft, def_opt)

let param_doc_re_str = {|@param[ ]+\([^ ]+\)[ ]+\(.+\)|}

let param_doc_re = Str.regexp param_doc_re_str

type param_info = {
  param_name: string;
  param_desc: string;
}

let parse_param_docs (line : string) : param_info option =
  if Str.string_match param_doc_re line 0 then
    let param_name = Str.matched_group 1 line in
    let param_desc = Str.matched_group 2 line in
    Some { param_name; param_desc }
  else
    None

(* convert all multiline @attribute descriptions to singleline ones *)
let parse_documentation_attributes (documentation_lines : string list) :
    string list =
  let attributes_info =
    List.fold
      ~init:([], None)
      ~f:(fun (documentation_attributes, current_attribute) line ->
        let trimmed_line = Caml.String.trim line in
        if Str.string_match (Str.regexp "@.*") trimmed_line 0 then
          (* new attribute *)
          match current_attribute with
          | Some current_attribute ->
            (* add previous attribute to the list and start building up this one *)
            ( List.cons current_attribute documentation_attributes,
              Some trimmed_line )
          | None -> (documentation_attributes, Some trimmed_line)
        else
          (* either continuing attribute or not related to attributes *)
          match current_attribute with
          | Some current_attribute ->
            let new_current_attribute =
              current_attribute ^ " " ^ trimmed_line
            in
            (documentation_attributes, Some new_current_attribute)
          | None -> (documentation_attributes, None))
      documentation_lines
  in
  match snd @@ attributes_info with
  | Some current_attribute ->
    (* add final attribute *)
    List.cons current_attribute (fst @@ attributes_info)
  | None -> fst @@ attributes_info

(* convert the documentation lines into a map of mentioned parameters -> their descriptions *)
let get_param_docs (documentation_lines : string list) :
    string Core_kernel.String.Map.t =
  let documentation_attributes =
    parse_documentation_attributes documentation_lines
  in
  List.fold
    ~init:String.Map.empty
    ~f:(fun param_docs line ->
      let split = parse_param_docs line in
      match split with
      | Some param_info ->
        let param_name =
          if String_utils.string_starts_with param_info.param_name "$" then
            param_info.param_name
          else
            "$" ^ param_info.param_name
        in
        Map.set ~key:param_name ~data:param_info.param_desc param_docs
      | None -> param_docs)
    documentation_attributes

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
  let (mode, tree) = Full_fidelity_ast.parse_text parser_env source_text in
  let results =
    Full_fidelity_ast.lower_tree parser_env source_text mode tree
  in
  let ast = results.Full_fidelity_ast.ast in
  let nast = Ast_to_nast.convert ast in
  let tast = ServerIdeUtils.check_ast tcopt nast in
  let offset = SourceText.position_to_offset source_text (line, column) in
  match
    get_positional_info
      (Full_fidelity_ast.PositionedSyntaxTree.root tree)
      offset
  with
  | None -> None
  | Some ((symbol_line, symbol_char), argument_idx) ->
    let results = IdentifySymbolService.go tast symbol_line symbol_char in
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
      | Some (occurrence, ft, def_opt) ->
        Typing_defs.(
          Lsp.SignatureHelp.(
            let tast_env = Tast_env.empty tcopt in
            let siginfo_label =
              Tast_env.print_ty_with_identity
                tast_env
                (DeclTy (Reason.Rnone, Tfun ft))
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
            let documentation_split =
              match siginfo_documentation with
              | Some doc -> Some (Str.split (Str.regexp "\n") doc)
              | None -> None
            in
            let param_docs =
              match documentation_split with
              | Some documentation_split ->
                Some (get_param_docs documentation_split)
              | None -> None
            in
            let params =
              List.map ft.ft_params ~f:(fun param ->
                  let parinfo_label =
                    match param.fp_name with
                    | Some s -> s
                    | None -> Tast_env.print_decl_ty tast_env ft.ft_ret.et_type
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
