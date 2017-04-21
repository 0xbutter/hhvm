(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *)

module CoroutineMethodLowerer = Coroutine_method_lowerer
module CoroutineSyntax = Coroutine_syntax
module EditableSyntax = Full_fidelity_editable_syntax
module List = Core_list
module Rewriter = Full_fidelity_rewriter.WithSyntax(EditableSyntax)
module SourceText = Full_fidelity_source_text
module SyntaxTree = Full_fidelity_syntax_tree

open CoroutineSyntax
open EditableSyntax

(**
 * If the provided methodish declaration is for a coroutine, rewrites the
 * methodish declaration into a desugared coroutine implementation.
 *)
let maybe_rewrite_method node =
  match syntax node with
  | MethodishDeclaration node ->
      (* TODO(tingley): Using the MethodishDeclaration, generate the coroutine
         state machine if the node is a coroutine. *)
      CoroutineMethodLowerer.maybe_rewrite_methodish_declaration node
  | _ ->
      (* Irrelevant input. *)
      None

(**
 * Accumulates node transforms.
 *
 * Transforms method nodes, accumulating all nodes. Accumulates whether at least
 * one node has been transformed.
 *)
let rewrite_classish_body_element (methods_acc, any_rewritten_acc) node =
  Option.value_map
    (maybe_rewrite_method node)
    ~default:(node :: methods_acc, any_rewritten_acc)
    ~f:(fun method_node -> method_node :: methods_acc, true)

(**
 * Rewrites classish body elements. If at least one element is modified, then
 * returns Some with all of the nodes. Otherwise, returns None.
 *)
let maybe_rewrite_classish_body_elements node =
  match syntax node with
  | SyntaxList syntax_list ->
      let rewritten_nodes, any_rewritten =
        List.fold
          ~f:rewrite_classish_body_element
          ~init:([], false)
          syntax_list in
      if any_rewritten then
        Some (make_list rewritten_nodes)
      else
        None
  | _ ->
      (* Missing, unexpected, or malformed input, so we won't transform the
         class. *)
      None

(**
 * Rewrites the elements of the body.
 *)
let maybe_rewrite_classish_body node =
  let make_syntax node = make_syntax (ClassishBody node) in
  match syntax node with
  | ClassishBody ({ classish_body_elements; _; } as node) ->
      Option.map
        (maybe_rewrite_classish_body_elements classish_body_elements)
        (fun classish_body_elements ->
          make_syntax { node with classish_body_elements; })
  | _ ->
      (* Unexpected or malformed input, so we won't transform the coroutine. *)
      None

(**
 * If the class contains at least one coroutine method, then those methods are
 * rewritten. Otherwise, the class is not transformed.
 *)
let maybe_rewrite_class node =
  let make_syntax node = make_syntax (ClassishDeclaration node) in
  match syntax node with
  | ClassishDeclaration ({ classish_body; _; } as class_node) ->
      Option.map
        (maybe_rewrite_classish_body classish_body)
        ~f:(fun classish_body ->
          make_syntax { class_node with classish_body; })
  | _ ->
      (* Irrelevant input. *)
      None

(**
 * Rewrites toplevel classes. The class nodes themselves are transformed
 * directly.
 *)
let rewrite_classes (node_acc, any_rewritten_acc) node =
  Option.value_map
    (maybe_rewrite_class node)
    ~default:(node :: node_acc, any_rewritten_acc)
    ~f:(fun node -> node :: node_acc, true)

(**
 * Rewrites classes containing coroutine methods.
 *)
let maybe_rewrite_syntax_list node =
  match syntax node with
  | SyntaxList syntax_list ->
      let rewritten_nodes, any_rewritten =
        List.fold
          ~f:rewrite_classes
          ~init:([], false)
          syntax_list in
      if any_rewritten then Some (make_list rewritten_nodes) else None
  | _ ->
      (* Irrelevant input. *)
      None

(**
 * Bridge function so that maybe_rewrite_class returns a value compatible with
 * the Rewriter.
 *)
let rewrite node =
  Some (
    Option.value_map
      (maybe_rewrite_syntax_list node)
      ~default:(node, false)
      ~f:(fun node -> node, true)
  )

let lower_coroutines syntax_tree =
  syntax_tree
    |> from_tree
    |> Rewriter.rewrite_pre rewrite
    |> fst
    |> text
    |> SourceText.make
    |> SyntaxTree.make
