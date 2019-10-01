(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open Typing_env_types

module StateErrors = struct
  module IdentMap = MyMap.Make (Ident)

  type t = unit IdentMap.t ref

  let mk_empty () = ref IdentMap.empty

  let add t id = t := IdentMap.add id () !t

  let has_error t id = IdentMap.mem id !t

  let show t m =
    print_endline m;
    IdentMap.iter (fun k _ -> print_endline (string_of_int k)) !t
end

let make_error_callback errors var _l1 _l2 = StateErrors.add errors var

module type MarshalledData = sig
  type t
end

module StateFunctor (M : MarshalledData) = struct
  type t = M.t

  let load path =
    let channel = In_channel.create path in
    let data : t = Marshal.from_channel channel in
    In_channel.close channel;
    data

  let save path t =
    let out_channel = Out_channel.create path in
    Marshal.to_channel out_channel t [];
    Out_channel.close out_channel
end

let folder_name = ".global_inference_artifacts/"

let artifacts_path : string ref = ref ""

module StateConstraintGraph = struct
  include StateFunctor (struct
    type t = env * StateErrors.t
  end)

  let merge_subgraph (env, errors) subgraph =
    let env =
      IMap.fold
        (fun var tyvar_info env ->
          let current_tyvar_info : tyvar_info_ =
            Typing_env.get_tyvar_info env var
          in
          let pos =
            match (tyvar_info.tyvar_pos, current_tyvar_info.tyvar_pos) with
            | (p, t) when t = Pos.none -> p
            | (_, p) -> p
          in
          let current_tyvar_info =
            {
              current_tyvar_info with
              appears_covariantly =
                tyvar_info.appears_covariantly
                || current_tyvar_info.appears_covariantly;
              appears_contravariantly =
                tyvar_info.appears_contravariantly
                || current_tyvar_info.appears_contravariantly;
              tyvar_pos = pos;
            }
          in
          (* We store in the env the updated tyvar_info_ *)
          Typing_env.update_tyvar_info env var current_tyvar_info
          (* Add the missing upper and lower bounds - and do the transitive closure *)
          |> TySet.fold
               (fun bound env ->
                 Typing_subtype.add_tyvar_upper_bound_and_close
                   env
                   var
                   bound
                   (make_error_callback errors var))
               tyvar_info.upper_bounds
          |> TySet.fold
               (fun bound env ->
                 Typing_subtype.add_tyvar_lower_bound_and_close
                   env
                   var
                   bound
                   (make_error_callback errors var))
               tyvar_info.lower_bounds)
        subgraph
        env
    in
    (env, errors)

  let merge_subgraphs (env, errors) subgraphs =
    List.fold ~f:merge_subgraph ~init:(env, errors) subgraphs
end

module StateSubConstraintGraphs = struct
  include StateFunctor (struct
    type t = global_tvenv list
  end)

  let save subcontraints =
    let subcontraints =
      List.filter ~f:(fun e -> not @@ IMap.is_empty e) subcontraints
    in
    if !artifacts_path = "" then
      artifacts_path := Filename.concat "/tmp" folder_name;
    if subcontraints = [] then
      ()
    else
      let path =
        Filename.concat
          !artifacts_path
          ("subgraph_" ^ string_of_int (Ident.tmp ()))
      in
      save path subcontraints
end

module StateSolvedGraph = struct
  include StateFunctor (struct
    type t = env * StateErrors.t * (Pos.t * int) list
  end)

  let save path t = save path t

  let from_constraint_graph (constraintgraph, errors) =
    let extract_pos = function
      | LocalTyvar tyvar -> tyvar.tyvar_pos
      | GlobalTyvar -> Pos.none
    in
    let positions =
      IMap.fold
        (fun var tyvar_info l -> (extract_pos tyvar_info, var) :: l)
        constraintgraph.tvenv
        []
    in
    let (_, tyvars) = List.unzip positions in
    let env = { constraintgraph with tyvars_stack = [(Pos.none, tyvars)] } in
    let env =
      Typing_solver.close_tyvars_and_solve_gi env (make_error_callback errors)
    in
    let env =
      Typing_solver.solve_all_unsolved_tyvars_gi
        env
        (make_error_callback errors)
    in
    (env, errors, positions)
end

let init () =
  let path = Filename.concat "/tmp" folder_name in
  Disk.rm_dir_tree path;
  if not @@ Disk.file_exists path then Disk.mkdir path 0o777
