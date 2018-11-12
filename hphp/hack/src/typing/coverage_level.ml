(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Ide_api_types
open Typing_defs
open Utils

module TUtils = Typing_utils
module Reason = Typing_reason

(* This should be configurable by client command args... eventually*)
let sample_rate = 0
let display_limit = 10
let samples_limit = 5

type result = (Pos.absolute * coverage_level) list

let string_of_level = function
  | Checked   -> "checked"
  | Partial   -> "partial"
  | Unchecked -> "unchecked"

module CLMap = MyMap.Make (struct
  type t = coverage_level
  let compare x y = Pervasives.compare x y
end)

type pos_stats_entry = {
  (* How many times this reason position has occured. *)
  pos_count : int;
  (* Random sample of expressions where this reason position has occured, for
   * debugging purposes *)
  samples : Pos.t list;
}

type level_stats_entry = {
  (* Number of expressions of this level *)
  count : int;
  (* string of reason -> position of reason -> stats *)
  reason_stats : (pos_stats_entry Pos.Map.t) SMap.t;
}

let empty_pos_stats_entry = {
  pos_count = 0;
  samples = [];
}

let empty_level_stats_entry = {
  count = 0;
  reason_stats = SMap.empty;
}

let empty_counter =
  let m = CLMap.empty in
  let m = CLMap.add Checked empty_level_stats_entry m in
  let m = CLMap.add Partial empty_level_stats_entry m in
  CLMap.add Unchecked empty_level_stats_entry m

(* This is highly unscientific and not really uniform sampling, but for
 * debugging purposes should be enough. *)
let merge_pos_stats_samples l1 l2 =
  let rec pick_n acc n m l =
    if n == 0 then acc
    else if m <= n then l @ acc else match l with
      | [] -> acc
      | h::tl ->
        if Random.int m < n then pick_n (h::acc) (n-1) (m-1) tl
        else pick_n acc n (m-1) tl in
  pick_n [] samples_limit ((List.length l1) + (List.length l2)) (l1 @ l2)

let add_sample_pos p samples =
  merge_pos_stats_samples samples [p]

let incr_reason_stats r p reason_stats =
  if sample_rate = 0 || Random.int sample_rate <> 0 then reason_stats else
  let reason_pos = Reason.to_pos r in
  let string_key = Reason.to_constructor_string r in
  let pos_stats_map = match SMap.get string_key reason_stats with
    | Some x -> x
    | None -> Pos.Map.empty in
  let pos_stats = match Pos.Map.get reason_pos pos_stats_map with
    | Some x -> x
    | None -> empty_pos_stats_entry in
  let pos_stats = {
    pos_count = pos_stats.pos_count + sample_rate;
    samples = add_sample_pos p pos_stats.samples
  } in
  SMap.add
    string_key
    (Pos.Map.add reason_pos pos_stats pos_stats_map)
    reason_stats

let incr_counter k (r, p, c) =
  let v = CLMap.find_unsafe k c in
  CLMap.add k {
    count = v.count + 1;
    reason_stats = incr_reason_stats r p v.reason_stats;
  } c

let merge_pos_stats p1 p2 = {
  pos_count = p1.pos_count + p2.pos_count;
  samples = merge_pos_stats_samples p1.samples p2.samples;
}

let merge_reason_stats s1 s2 =
  SMap.merge (fun _ s1 s2 ->
    Option.merge s1 s2 (fun s1 s2 ->
      Pos.Map.merge (fun _ p1 p2 ->
        Option.merge p1 p2 (fun p1 p2 ->
          merge_pos_stats p1 p2
        )
      ) s1 s2
    )
  ) s1 s2

let merge_and_sum cs1 cs2 =
  CLMap.merge (fun _ c1 c2 ->
    Option.merge c1 c2 (fun c1 c2 -> {
      count = c1.count + c2.count;
      reason_stats = merge_reason_stats c1.reason_stats c2.reason_stats
    })
  ) cs1 cs2

(* An assoc list that counts the number of expressions at each coverage level,
 * along with stats about their reasons *)
type level_stats = level_stats_entry CLMap.t

(* There is a trie in utils/, but it is not quite what we need ... *)

type 'a trie =
  | Leaf of 'a
  | Node of 'a * 'a trie SMap.t

let rec is_tany ty = match ty with
  | r, (Tany | Terr) -> Some r
  | _, Tunresolved [] -> None
  | _, Tunresolved (h::tl) -> begin match is_tany h with
    | Some r when
      List.for_all tl (compose (Option.is_some) (is_tany)) -> Some r
    | _ -> None
    end
  | _ -> None

let level_of_type fixme_map (p, ty) =
  let r, lvl = match ty with
    | r, Tobject -> r, Partial
    | r, _ -> match is_tany ty with
      | Some r -> r, Unchecked
      | None -> match TUtils.HasTany.check_why ty with
        | Some r -> r, Partial
        | _ -> r, Checked in
  let line = Pos.line p in
  (* If the line has a HH_FIXME, then mark it as (at most) partially checked *)
  match lvl with
  | Checked when IMap.mem line fixme_map ->
      r, Partial
  | Unchecked | Partial | Checked -> r, lvl

let level_of_type_mapper fn =
  let fixme_map = Fixmes.HH_FIXMES.find_unsafe fn in
  level_of_type fixme_map

(* Coverage analysis will return multiple overlapping ranges, e.g. for *)
(* "$user = $vc->getUserID()" it can report three uncovered ranges:    *)
(* "$vc" and "$vc->getUserID()" and "$user = $vc->getUserID()".        *)
(* It will be most actionable for the user to show the smallest range  *)
(* only, i.e. the root cause of all these cascading reports.           *)
let merge_adjacent_results (results: result) : result =
  (* Imagine a tree of uncovered spans based on range inclusion. *)
  (* This sorted function gives a pre-order flattening of that tree, *)
  (* by sorting forwards on start and then backwards on end. *)
  let pos_compare_by_inclusion x y =
    let (xstart, xend), (ystart, yend) = Pos.info_raw x, Pos.info_raw y in
    let r = xstart - ystart in if r <> 0 then r else yend - xend
  in
  let filter_sort_merge (filter: coverage_level) (results: result) : result =
    let filtered = Core_list.filter_map results ~f:(fun (loc, level) ->
      if level = filter then Some loc else None) in
    let sorted = Core_list.sort filtered ~cmp:pos_compare_by_inclusion in
    (* We can use that sorted list to remove any span which contains another, so *)
    (* the user only sees actionable reports of the smallest causes of untypedness. *)
    (* The algorithm: accept a range if its immediate successor isn't contained by it. *)
    let f (candidate, acc) loc =
      if Pos.contains candidate loc then (loc, acc) else (loc, candidate :: acc) in
    let merged = match sorted with
      | [] -> []
      | (first::_) ->
        let (final_candidate, singles) = Core_list.fold sorted ~init:(first,[]) ~f in
        final_candidate :: singles in
    Core_list.map merged ~f:(fun loc -> (loc, filter))
  in
  (filter_sort_merge Checked results)
  @ (filter_sort_merge Unchecked results)
  @ (filter_sort_merge Partial results)
