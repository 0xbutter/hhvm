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
open Hhbc_ast

let literal_from_bool b =
  if b then True else False

let literal_from_binop op left right =
  (* TODO: HHVM does not allow 2+2 in an attribute, but does allow it in
  a constant initializer. It seems reasonable to allow this in attributes
  as well. Make sure this decision is documented in the specification. *)
  (* TODO: Deal with integer overflow *)
  match (op, left, right) with
  | (Ast.Eqeq, Int left, Int right)
  | (Ast.EQeqeq, Int left, Int right) ->
    literal_from_bool (Int64.equal left right)
  | (Ast.Diff, Int left, Int right)
  | (Ast.Diff2, Int left, Int right) ->
    literal_from_bool (not (Int64.equal left right))
  | (Ast.Lt, Int left, Int right) ->
    literal_from_bool ((Int64.compare left right) < 0)
  | (Ast.Lte, Int left, Int right) ->
    literal_from_bool ((Int64.compare left right) <= 0)
  | (Ast.Gt, Int left, Int right) ->
    literal_from_bool ((Int64.compare left right) > 0)
  | (Ast.Gte, Int left, Int right) ->
    literal_from_bool ((Int64.compare left right) >= 0)

  | (Ast.Amp, Int left, Int right) -> Int (Int64.logand left right)
  | (Ast.Bar, Int left, Int right) -> Int (Int64.logor left right)
  | (Ast.Xor, Int left, Int right) -> Int (Int64.logxor left right)
  | (Ast.BArbar, Int left, Int right) ->
    if Int64.equal left Int64.zero then
      literal_from_bool (not (Int64.equal right Int64.zero))
    else
      True
  | (Ast.AMpamp, Int left, Int right) ->
    if Int64.equal left Int64.zero then
      False
    else
      literal_from_bool (not (Int64.equal right Int64.zero))
  | (Ast.Ltlt, Int left, Int right) ->
    (* TODO: Deal with overflow of shifter *)
    let right = Int64.to_int right in
    Int (Int64.shift_left left right)
  | (Ast.Gtgt, Int left, Int right) ->
    (* TODO: Deal with overflow of shifter *)
    let right = Int64.to_int right in
    Int (Int64.shift_right left right)

  | (Ast.Dot, Int left, Int right) ->
    String ((Int64.to_string left) ^ (Int64.to_string right))

  | (Ast.Plus, Int left, Int right) -> Int (Int64.add left right)
  | (Ast.Minus, Int left, Int right) -> Int (Int64.sub left right)
  | (Ast.Star, Int left, Int right) -> Int (Int64.mul left right)
  (* TODO: StarStar has special behaviour *)
  | (Ast.Slash, Int left, Int right) ->
    (* TODO: Deal with div by zero *)
    if (Int64.rem left right) = Int64.zero then
      Int (Int64.div left right)
    else
      let left = Int64.to_float left in
      let right = Int64.to_float right in
      let quotient = left /. right in
      (* TODO: Consider making double take a float, not a string. *)
      (* TODO: The original HHVM emitter produces considerably more precision
      when printing out floats *)
      Double (string_of_float quotient)
  | (Ast.Percent, Int left, Int right) ->
    (* TODO: Deal with div by zero *)
    Int (Int64.rem left right)
  | _ -> failwith "Binary operation not yet implemented on literals"

let rec collection_literal_fields fields =
  let folder (index, consts) field =
    match field with
    | Ast.AFvalue v ->
      (index + 1, literal_from_expr v :: Int (Int64.of_int index) :: consts)
    | Ast.AFkvalue (k, v) ->
      (index, literal_from_expr k :: literal_from_expr v :: consts )
  in
  List.rev @@ snd @@ List.fold_left fields ~init:(0, []) ~f:folder

and dictionary_literal fields =
  let num = List.length fields in
  let fields = collection_literal_fields fields in
  Dict (num, fields)

and array_literal fields =
  let num = List.length fields in
  let fields = collection_literal_fields fields in
  Array (num, fields)

and literal_from_expr expr =
  match snd expr with
  | Ast.Float (_, litstr) -> Double litstr
  | Ast.String (_, litstr) -> String litstr
  | Ast.Int (_, litstr) -> Int (Int64.of_string litstr)
  | Ast.Null -> Null
  | Ast.False -> False
  | Ast.True -> True
  | Ast.Array fields -> array_literal fields
  | Ast.Collection ((_, "dict"), fields) -> dictionary_literal fields
  (* TODO: Vec, Keyset, etc. *)
  | Ast.Binop (op, left, right) ->
    let left = literal_from_expr left in
    let right = literal_from_expr right in
    literal_from_binop op left right
  (* TODO: Unary ops *)
  (* TODO: HHVM does not allow <<F(2+2)>> in an attribute, but Hack does, and
   this seems reasonable to allow. Right now this will crash if given an
   expression rather than a literal in here.  In particular, see what unary
   minus does; do we allow it on a literal int? We should. *)
   | _ -> failwith "Expected a literal expression"

let literals_from_exprs_with_index exprs =
 List.rev @@ snd @@
 List.fold_left
   exprs
   ~init:(0, [])
   ~f:(fun (index, l) e ->
     (index + 1, literal_from_expr e :: Int (Int64.of_int index) :: l))
