(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

module CoroutineMethodLowerer = Coroutine_method_lowerer
module CoroutineStateMachineGenerator = Coroutine_state_machine_generator
module CoroutineSyntax = Coroutine_syntax
module CoroutineTypeLowerer = Coroutine_type_lowerer
module EditableSyntax = Full_fidelity_editable_syntax
module EditableToken = Full_fidelity_editable_token
module List = Core_list
module Rewriter = Full_fidelity_rewriter.WithSyntax(EditableSyntax)
module SourceText = Full_fidelity_source_text
module SyntaxTree = Full_fidelity_syntax_tree

open CoroutineSyntax
open EditableSyntax
open Coroutine_type_lowerer

(**
 * Rewrites coroutine annotations.
 *
 * The following:
 *
 *   public function returnVoidVoidCoroutineLambda(
 *   ): (coroutine function(): void) { ... }
 *
 * Will be rewritten into:
 *
 *   public function returnVoidVoidCoroutineLambda(
 *   ): (function(
 *     CoroutineContinuation<CoroutineUnit>
 *   ): CoroutineResult<CoroutineUnit>) { ... }
 *
 * The following:
 *
 *   public function returnIntIntCoroutineLambda(
 *   ): (coroutine function(int): int) { ... }
 *
 * Will be rewritten into:
 *
 *    public function returnIntIntCoroutineLambda(
 *    ): (function(
 *      CoroutineContinuation<int>,
 *      int,
 *    ): CoroutineResult<int>) { ... }
 *)
let maybe_rewrite_coroutine_annotations node =
  match syntax node with
  | ClosureTypeSpecifier ({
      closure_coroutine = {
        syntax = Token { EditableToken.kind = TokenKind.Coroutine; _; };
        _;
      };
      closure_parameter_types;
      closure_return_type;
      _;
    } as node) ->
      Rewriter.Result.Replace (
        let closure_return_type =
          CoroutineTypeLowerer.rewrite_return_type closure_return_type in
        make_syntax (
          ClosureTypeSpecifier {
            node with
              closure_coroutine = make_missing ();
              closure_parameter_types =
                prepend_to_comma_delimited_syntax_list
                  (make_continuation_type_syntax closure_return_type)
                  closure_parameter_types;
              closure_return_type =
                make_coroutine_result_type_syntax closure_return_type;
          }
        )
      )
  | _ -> Rewriter.Result.Keep

(**
 * If the function declaration is for a coroutine, rewrites the declaration
 * header and the function body into a desugared coroutine implementation.
 * Also extracts the coroutine's closure.
 *)
let maybe_rewrite_classish_body_element class_node classish_body_element_node =
  match syntax classish_body_element_node with
  | MethodishDeclaration ({
      methodish_function_decl_header = {
        syntax = FunctionDeclarationHeader ({
          function_coroutine;
          _;
        } as header_node);
        _;
      };
      _;
    } as method_node) when not @@ is_missing function_coroutine ->
      (* TODO: We need to rewrite non-coroutine functions if they contain
         coroutine lambdas. *)
      let header_node = rewrite_header_node header_node in
      let rewritten_body, closure_syntax =
        CoroutineStateMachineGenerator.generate_coroutine_state_machine
          class_node
          method_node
          header_node in
      let rewritten_method_syntax =
        CoroutineMethodLowerer.rewrite_methodish_declaration
          class_node
          method_node
          header_node
          rewritten_body in
      Some (rewritten_method_syntax, closure_syntax)
  | _ ->
      (* Irrelevant input. *)
      None

let compute_body_nodes class_node =
  let gather_rewritten_syntaxes
      classish_body_element_node
      (classish_body_elements_acc, closures_acc, any_rewritten_acc) =
    Option.value_map
      (maybe_rewrite_classish_body_element
        class_node
        classish_body_element_node)
      ~default:
        (classish_body_element_node :: classish_body_elements_acc,
          closures_acc,
          any_rewritten_acc)
      ~f:(fun (rewritten_method_syntax, closure_syntax) ->
          rewritten_method_syntax :: classish_body_elements_acc,
          closure_syntax :: closures_acc,
          true) in
  List.fold_right ~f:gather_rewritten_syntaxes ~init:([], [], false)

(**
 * Rewrites classish body elements. If at least one element is modified, then
 * returns Some with all of the nodes. Otherwise, returns None.
 *)
let maybe_rewrite_classish_body_elements
    class_node
    classish_body_elements_node =
  let rewritten_nodes, closure_nodes, any_rewritten =
    classish_body_elements_node
      |> syntax_node_to_list
      |> compute_body_nodes class_node in
  Option.some_if any_rewritten (make_list rewritten_nodes, closure_nodes)

(**
 * If the class contains at least one coroutine method, then those methods are
 * rewritten, and closures are generated as necessary. Otherwise, the
 * class is not transformed.
 *)
let maybe_rewrite_class node =
  (* TODO: We need to rewrite top-level coroutine methods *)
  (* TODO: Do we need to rewrite top-level statements that contain coroutine
  lambdas? *)
  match syntax node with
  | ClassishDeclaration ({
      classish_body = {
        syntax = ClassishBody ({
          classish_body_elements;
          _;
        } as classish_body_node);
        _;
      };
      _;
    } as class_node) ->
      Option.map
        (maybe_rewrite_classish_body_elements class_node classish_body_elements)
        ~f:(fun (classish_body_elements, closure_nodes) ->
          make_syntax @@ ClassishDeclaration {
            class_node with classish_body = make_syntax @@ ClassishBody {
              classish_body_node with classish_body_elements;
            };
          },
          closure_nodes)
  | _ ->
      (* Irrelevant input. *)
      None

(**
 * Rewrites toplevel classes. The class nodes themselves are transformed
 * directly. Generated closure nodes are collected and written alongside
 * of the class nodes.
 *)
let rewrite_classes node (node_acc, any_rewritten_acc) =
  Option.value_map
    (maybe_rewrite_class node)
    ~default:(node :: node_acc, any_rewritten_acc)
    ~f:(fun (node, closure_nodes) ->
      node :: (closure_nodes @ node_acc), true)

(**
 * Rewrites classes containing coroutine methods. Additional closure classes
 * may be generated alongside of the original class.
 *)
let maybe_rewrite_syntax_list node =
  match syntax node with
  | SyntaxList syntax_list ->
      let rewritten_nodes, any_rewritten =
        List.fold_right
          ~f:rewrite_classes
          ~init:([], false)
          syntax_list in
      if any_rewritten then
        Rewriter.Result.Replace (make_list rewritten_nodes)
      else
        Rewriter.Result.Keep
  | _ ->
      (* Irrelevant input. *)
      Rewriter.Result.Keep

let lower_coroutines syntax_tree =
  syntax_tree
    |> from_tree
    |> Rewriter.rewrite_post maybe_rewrite_coroutine_annotations
    |> Rewriter.rewrite_post maybe_rewrite_syntax_list
    |> text
    |> SourceText.make
    |> SyntaxTree.make
