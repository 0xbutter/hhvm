(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Direct_decl_parser

type verbosity =
  | Standard
  | Verbose
  | Silent

let init () =
  Relative_path.(set_path_prefix Root (Path.make "/"));
  let (_handle : SharedMem.handle) =
    SharedMem.init ~num_workers:0 GlobalConfig.default_sharedmem_config
  in
  Parser_options_provider.set ParserOptions.default;
  GlobalNamingOptions.set TypecheckerOptions.default

let compare_decl verbosity fn =
  let fn =
    if Filename.is_relative fn then
      Path.concat (Path.getcwd ()) fn |> Path.to_string
    else
      fn
  in
  let text = RealDisk.cat fn in
  let fn = Relative_path.(create Root fn) in
  let decls =
    Result.ok_or_failwith (parse_decls ~trace:(verbosity = Verbose) fn)
  in
  let facts =
    Option.value_exn
      ~message:"Could not parse facts from file"
      (Facts_parser.from_text
         ~php5_compat_mode:false
         ~hhvm_compat_mode:false
         ~disable_nontoplevel_declarations:false
         ~disable_legacy_soft_typehints:false
         ~allow_new_attribute_syntax:true
         ~disable_legacy_attribute_syntax:false
         ~filename:fn
         ~text)
  in
  let passes_symbol_check () =
    let compare name facts_symbols decl_symbols =
      let facts_symbols = SSet.of_list facts_symbols in
      let decl_symbols = SSet.of_list decl_symbols in
      let facts_only = SSet.diff facts_symbols decl_symbols in
      let decl_only = SSet.diff decl_symbols facts_symbols in
      if (not @@ SSet.is_empty facts_only) || (not @@ SSet.is_empty decl_only)
      then (
        if not @@ SSet.is_empty facts_only then
          Printf.eprintf
            "The following %s were found in the facts parse but not the decl parse: %s\n"
            name
            (SSet.show facts_only);
        if not @@ SSet.is_empty decl_only then
          Printf.eprintf
            "The following %s were found in the decl parse but not the facts parse: %s\n"
            name
            (SSet.show decl_only);
        prerr_endline "";
        false
      ) else
        true
    in
    [
      compare "typedef(s)" facts.Facts.type_aliases (SMap.keys decls.typedefs);
      compare "constant(s)" facts.Facts.constants (SMap.keys decls.consts);
      compare "function(s)" facts.Facts.functions (SMap.keys decls.funs);
      compare
        "class(es)"
        (Facts.InvSMap.keys facts.Facts.types)
        (SMap.keys decls.classes);
    ]
    |> List.reduce_exn ~f:( && )
  in
  let passes_decl_check () =
    let old_decls =
      {
        classes = SMap.empty;
        funs = SMap.empty;
        consts = SMap.empty;
        typedefs = SMap.empty;
      }
    in
    let old_decls =
      List.fold
        facts.Facts.constants
        ~init:old_decls
        ~f:(fun old_decls const ->
          {
            old_decls with
            consts =
              SMap.add
                const
                (fst (Decl.declare_const_in_file fn ("\\" ^ const)))
                old_decls.consts;
          })
    in
    (* These can't be compared directly because the parsed decls are allocated into a Rust arena
       that doesn't live in the OCaml major heap, so it refuses to compare them using polymorphic
       compare. *)
    if
      Direct_decl_parser.show_decls decls
      = Direct_decl_parser.show_decls old_decls
    then
      true
    else (
      Printf.eprintf
        "Decls differed. Parsed decls:\n%s\n\nGenerated decls:\n%s"
        (show_decls decls)
        (show_decls old_decls);
      false
    )
  in
  passes_symbol_check () && passes_decl_check ()

type modes = CompareDirectDeclParser

let () =
  let usage =
    Printf.sprintf "Usage: %s [OPTIONS] mode filename\n" Sys.argv.(0)
  in
  let usage_and_exit () =
    prerr_endline usage;
    exit 1
  in
  let mode = ref None in
  let set_mode m () =
    match !mode with
    | None -> mode := Some m
    | Some _ -> usage_and_exit ()
  in
  let file = ref None in
  let set_file f =
    match !file with
    | None -> file := Some f
    | Some _ -> usage_and_exit ()
  in
  let verbosity = ref Standard in
  Arg.parse
    [
      ( "--compare-direct-decl-parser",
        Arg.Unit (set_mode CompareDirectDeclParser),
        "(mode) Runs the direct decl parser against the FFP -> naming -> decl pipeline and compares their output"
      );
      ( "--verbosity",
        Arg.Symbol
          ( ["silent"; "standard"; "verbose"],
            fun v ->
              verbosity :=
                match v with
                | "silent" -> Silent
                | "standard" -> Standard
                | "verbose" -> Verbose
                | _ ->
                  failwith
                  @@ Printf.sprintf "Did not understand verbosity level %s" v
          ),
        " Set the verbosity level. Silent will hide the \"no differences\" message on a successful "
        ^ "run, and verbose will print debugging information to the console" );
    ]
    set_file
    usage;
  match !mode with
  | None -> usage_and_exit ()
  | Some CompareDirectDeclParser ->
    begin
      match !file with
      | None -> usage_and_exit ()
      | Some file ->
        init ();
        if compare_decl !verbosity file then
          if !verbosity <> Silent then
            print_endline "They matched!"
          else
            ()
        else
          exit 1
    end
