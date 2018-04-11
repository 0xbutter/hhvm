(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Reordered_argument_collections
open Typing_defs
open Utils
open String_utils
include AutocompleteTypes

module Phase = Typing_phase
module TUtils = Typing_utils

let ac_env = ref None
let autocomplete_results : autocomplete_result list ref = ref []
let autocomplete_is_complete : bool ref = ref true

(* The position we're autocompleting at. This is used when computing completions
 * for global identifiers. *)
let (auto_complete_pos: Pos.t option ref) = ref None

type autocomplete_type =
  | Acid
  | Acnew
  | Actype
  | Acclass_get
  | Acprop

let (argument_global_type: autocomplete_type option ref) = ref None
let auto_complete_for_global = ref ""

let auto_complete_suffix = "AUTO332"
let suffix_len = String.length auto_complete_suffix
let strip_suffix s = String.sub s 0 (String.length s - suffix_len)
let is_auto_complete x =
  if !autocomplete_results = []
  then begin
    String.length x >= suffix_len &&
    let suffix = String.sub x (String.length x - suffix_len) suffix_len in
    suffix = auto_complete_suffix
  end else
    false

let autocomplete_result_to_json res =
  let func_param_to_json param =
    Hh_json.JSON_Object [ "name", Hh_json.JSON_String param.param_name;
                     "type", Hh_json.JSON_String param.param_ty;
                     "variadic", Hh_json.JSON_Bool param.param_variadic;
                   ]
  in
  let func_details_to_json details =
    match details with
     | Some fd -> Hh_json.JSON_Object [
           "min_arity", Hh_json.int_ fd.min_arity;
           "return_type", Hh_json.JSON_String fd.return_ty;
           "params", Hh_json.JSON_Array (List.map fd.params func_param_to_json);
       ]
     | None -> Hh_json.JSON_Null
  in
  let name = res.res_name in
  let pos = res.res_pos in
  let ty = res.res_ty in
  Hh_json.JSON_Object [
      "name", Hh_json.JSON_String name;
      "type", Hh_json.JSON_String ty;
      "pos", Pos.json pos;
      "func_details", func_details_to_json res.func_details;
      "expected_ty", Hh_json.JSON_Bool false; (* legacy field, left here in case clients need it *)
  ]

let get_partial_result name ty kind =
  Partial { ty; name; kind_=kind; }

let add_res (res: autocomplete_result) : unit =
  autocomplete_results := res :: !autocomplete_results

let add_partial_result name ty kind =
  add_res (get_partial_result name ty kind)

let autocomplete_token ac_type env x =
  if is_auto_complete (snd x)
  then begin
    ac_env := env;
    auto_complete_pos := Some (fst x);
    argument_global_type := Some ac_type;
    auto_complete_for_global := snd x
  end

let autocomplete_id id env = autocomplete_token Acid (Some env) id

let autocomplete_hint = autocomplete_token Actype None

let autocomplete_new cid env _ =
  match cid with
  | Nast.CI (sid, _) -> autocomplete_token Acnew (Some env) sid
  | _ -> ()

let get_class_elt_types env class_ cid elts =
  let elts = SMap.filter elts begin fun _ x ->
    Typing_visibility.is_visible env x.ce_visibility cid class_
  end in
  SMap.map elts (fun { ce_type = lazy ty; _ } -> ty)

let autocomplete_method is_static class_ ~targs:_ ~pos_params:_ id env cid
    ~is_method:_ ~is_const:_ =
  (* This is used for instance "$x->|" and static "Class1::|" members. *)
  (* It's also used for "<nt:fb:text |" XHP attributes, in which case  *)
  (* class_ is ":nt:fb:text" and its attributes are in tc_props.       *)
  if is_auto_complete (snd id)
  then begin
    ac_env := Some env;
    auto_complete_pos := Some (fst id);
    argument_global_type := Some Acclass_get;
    let add kind name ty = add_partial_result name (Phase.decl ty) kind in
    if is_static then begin
      SMap.iter (get_class_elt_types env class_ cid class_.tc_smethods) ~f:(add Method_kind);
      SMap.iter (get_class_elt_types env class_ cid class_.tc_sprops) ~f:(add Property_kind);
      SMap.iter (class_.tc_consts) ~f:(fun name cc -> add Class_constant_kind name cc.cc_type);
    end else begin
      SMap.iter (get_class_elt_types env class_ cid class_.tc_methods) ~f:(add Method_kind);
      SMap.iter (get_class_elt_types env class_ cid class_.tc_props) ~f:(add Property_kind);
    end
  end

let autocomplete_smethod = autocomplete_method true

let autocomplete_cmethod = autocomplete_method false

let autocomplete_lvar id env =
  (* This is used for "$|" and "$x = $|" local variables. *)
  if is_auto_complete (Local_id.get_name (snd id))
  then begin
    argument_global_type := Some Acprop;
    (* The typechecker might call this hook more than once (loops) so we
     * need to clear the list of results first or we could have repeat locals *)
    autocomplete_results := [];
    ac_env := Some env;
    auto_complete_pos := Some (fst id);
    (* Get the types of all the variables in scope at this point *)
    Local_id.Map.iter begin fun x (ty, _) ->
      add_partial_result (Local_id.get_name x) (Phase.locl ty) Variable_kind
    end (Typing_env.get_locals env);
  end

let should_complete_class completion_type class_kind =
  match completion_type, class_kind with
  | Some Acid, Some Ast.Cnormal
  | Some Acid, Some Ast.Cabstract
  | Some Acnew, Some Ast.Cnormal
  | Some Actype, Some _ -> true
  | _ -> false

let should_complete_fun completion_type =
  completion_type=Some Acid

let get_constructor_ty c =
  let pos = c.Typing_defs.tc_pos in
  let reason = Typing_reason.Rwitness pos in
  let return_ty = reason, Typing_defs.Tapply ((pos, c.Typing_defs.tc_name), []) in
  match (fst c.Typing_defs.tc_construct) with
    | Some elt ->
        begin match elt.ce_type with
          | lazy (_ as r, Tfun fun_) ->
              (* We have a constructor defined, but the return type is void
               * make it the object *)
              let fun_ = { fun_ with Typing_defs.ft_ret = return_ty } in
              r, Tfun fun_
          | _ -> (* how can a constructor not be a function? *) assert false
        end
    | None ->
        (* Nothing defined, so we need to fake the entire constructor *)
      reason,
      Typing_defs.Tfun
        (Typing_env.make_ft pos Nonreactive (*is_coroutine*)false [] return_ty)

(* Global identifier autocomplete uses search service to find matching names *)
let search_funs_and_classes input ~limit ~on_class ~on_function =
  HackSearchService.MasterApi.query_autocomplete input ~limit
    ~filter_map:begin fun _ _ res ->
      let name = res.SearchUtils.name in
      match res.SearchUtils.result_type with
      | HackSearchService.Class _-> on_class name
      | HackSearchService.Function -> on_function name
      | _ -> None
    end

(* compute_complete_global: given the sets content_funs and content_classes  *)
(* of function names and classes in the current file, returns a list of all  *)
(* possible identifier autocompletions at the autocomplete position (which   *)
(* is stored in a global mutable reference). The results are stored in the   *)
(* global mutable reference 'autocomplete_results'.                          *)
(* This function has two modes of dealing with namespaces...                 *)
(*     delimit_on_namespaces=true   delimit_on_namespaces=false              *)
(*      St| =>               Str                          Str\compare, ...   *)
(*  Str\\c| =>               compare                      Str\compare        *)
(* Essentially, 'delimit_on_namespaces=true' means that autocomplete treats  *)
(* namespaces as first class entities; 'false' means that it treats them     *)
(* purely as part of long identifier names where the symbol '\\' is not      *)
(* really any different from the symbol '_' for example.                     *)
(*                                                                           *)
(* XHP note:                                                                 *)
(* This function is also called for "<foo|", with gname="foo".               *)
(* This is a Hack shorthand for referring to the global classname ":foo".    *)
(* Our global dictionary of classnames stores the authoritative name ":foo", *)
(* and inside autocompleteService we do the job of inserting a leading ":"   *)
(* from the user's prefix, and stripping the leading ":" when we emit.       *)
let compute_complete_global
  ~(tcopt: TypecheckerOptions.t)
  ~(delimit_on_namespaces: bool)
  ~(autocomplete_context: AutocompleteTypes.legacy_autocomplete_context)
  ~(content_funs: Reordered_argument_collections.SSet.t)
  ~(content_classes: Reordered_argument_collections.SSet.t)
  : unit =
  let completion_type = !argument_global_type in
  let gname = Utils.strip_ns !auto_complete_for_global in
  let gname = strip_suffix gname in
  let gname = if autocomplete_context.is_xhp_classname then (":" ^ gname) else gname in
  (* Colon hack: in "case Foo::Bar:", we wouldn't want to show autocomplete *)
  (* here, but we would in "<nt:" and "$a->:" and "function f():". We can   *)
  (* recognize this case by whether the prefix is empty.                    *)
  if autocomplete_context.is_after_single_colon && gname = "" then
    ()
  else begin

    (* Objective: given "fo|", we should suggest functions in both the current *)
    (* namespace and also in the global namespace. We'll use gname_gns to      *)
    (* indicate what prefix to look for (if any) in the global namespace...    *)

    (* "$z = S|"      => gname = "S",      gname_gns = Some "S"   *)
    (* "$z = Str|"    => gname = "Str",    gname_gns = Some "Str" *)
    (* "$z = Str\\|"  => gname = "Str\\",  gname_gns = None       *)
    (* "$z = Str\\s|" => gname = "Str\\s", gname_gns = None       *)
    (* "$z = \\s|"    => gname = "\\s",    gname_gns = None       *)
    (* "...; |"       => gname = "",       gname_gns = Some ""    *)
    let gname_gns = if should_complete_fun completion_type then
      (* Disgusting hack alert!
       *
       * In PHP/Hack, namespaced function lookup falls back into the global
       * namespace if no function in the current namespace exists. The
       * typechecker knows everything that exists, and resolves all of this
       * during naming -- meaning that by the time that we get to typing, not
       * only has "gname" been fully qualified, but we've lost whatever it
       * might have looked like originally. This makes it tough to do the full
       * namespace fallback behavior here -- we'd like to know if whatever
       * "gname" corresponds to in the source code has a '\' to qualify it, but
       * since it's already fully qualified here, we can't know.
       *
       *   [NOTE(ljw): is this really even true? if user wrote "foo|" then name
       *   lookup of "fooAUTO332" is guaranteed to fail in the current namespace,
       *   and guaranteed to fail in the global namespace, so how would the
       *   typechecker fully qualify it? to what? Experimentally I've only
       *   ever seen gname to be the exact string that the user typed.]
       *
       * Except, we can kinda reverse engineer and figure it out. We have the
       * positional information, which we can use to figure out how long the
       * original source code token was, and then figure out what portion of
       * "gname" that corresponds to, and see if it has a '\'. Since fully
       * qualifying a name will always prepend, this all works.
       *)
      match !auto_complete_pos with
        | None -> None
        | Some p ->
            let len = (Pos.length p) - suffix_len in
            let start = String.length gname - len in
            if start < 0 || String.contains_from gname start '\\'
            then None else Some (strip_all_ns gname)
      else None in

    let does_fully_qualified_name_match_prefix ?(funky_gns_rules=false) name =
      let stripped_name = strip_ns name in
      if delimit_on_namespaces then
        (* name must match gname, and have no additional namespace slashes, e.g. *)
        (* name="Str\\co" gname="S" -> false *)
        (* name="Str\\co" gname="Str\\co" -> true *)
        string_starts_with stripped_name gname &&
          not (String.contains_from stripped_name (String.length gname) '\\')
      else
        match gname_gns with
        | _ when string_starts_with stripped_name gname -> true
        | Some gns when funky_gns_rules -> string_starts_with stripped_name gns
        | _ -> false
    in

    let string_to_replace_prefix name =
      let stripped_name = strip_ns name in
      if delimit_on_namespaces then
        (* returns the part of 'name' after its rightmost slash *)
        try
          let len = String.length stripped_name in
          let i = (String.rindex stripped_name '\\') + 1 in
          String.sub stripped_name i (len - i)
        with _ ->
          stripped_name
      else
        stripped_name
    in

    let result_count = ref 0 in

    let on_class name ~seen =
      if SSet.mem seen name then None else
      if not (does_fully_qualified_name_match_prefix name) then None else
      let target = Typing_lazy_heap.get_class tcopt name in
      let target_kind = Option.map target ~f:(fun c -> c.Typing_defs.tc_kind) in
      if not (should_complete_class completion_type target_kind) then None else
      Option.map target ~f:(fun c ->
        incr result_count;
        if completion_type = Some Acnew then
          get_partial_result
            (string_to_replace_prefix name) (Phase.decl (get_constructor_ty c)) Constructor_kind
        else
          let kind = match c.Typing_defs.tc_kind with
            | Ast.Cabstract -> Abstract_class_kind
            | Ast.Cnormal -> Class_kind
            | Ast.Cinterface -> Interface_kind
            | Ast.Ctrait -> Trait_kind
            | Ast.Cenum -> Enum_kind
          in
          let ty =
            Typing_reason.Rwitness c.Typing_defs.tc_pos,
            Typing_defs.Tapply ((c.Typing_defs.tc_pos, name), []) in
          get_partial_result (string_to_replace_prefix name) (Phase.decl ty) kind
      )
    in

    let on_function name ~seen =
      if autocomplete_context.is_xhp_classname then None else
      if SSet.mem seen name then None else
      if not (should_complete_fun completion_type) then None else
      if not (does_fully_qualified_name_match_prefix ~funky_gns_rules:true name) then None else
      Option.map (Typing_lazy_heap.get_fun tcopt name) ~f:(fun fun_ ->
        incr result_count;
        let ty = Typing_reason.Rwitness fun_.Typing_defs.ft_pos, Typing_defs.Tfun fun_ in
        get_partial_result (string_to_replace_prefix name) (Phase.decl ty) Function_kind
      )
    in

    let on_namespace name : autocomplete_result option =
      (* name will have the form "Str" or "HH\\Lib\\Str" *)
      (* Our autocomplete will show up in the list as "Str". *)
      if autocomplete_context.is_xhp_classname then None else
      if not delimit_on_namespaces then None else
      if not (does_fully_qualified_name_match_prefix name) then None else
      Some (Complete {
        res_pos = Pos.none |> Pos.to_absolute;
        res_ty = "namespace";
        res_name = string_to_replace_prefix name;
        res_kind = Namespace_kind;
        func_details = None;
      })
    in

    (* Try using the names in local content buffer first *)
    List.iter
      (List.filter_map (SSet.elements content_classes) (on_class ~seen:SSet.empty))
        add_res;
    List.iter
      (List.filter_map (SSet.elements content_funs) (on_function ~seen:SSet.empty))
        add_res;

    (* Add namespaces. The hack server doesn't index namespaces themselves; it  *)
    (* only stores names of functions and classes in fully qualified form, e.g. *)
    (*   \\HH\\Lib\\Str\\length                                                 *)
    (* If the project's .hhconfig has auto_namesspace_map "Str": "HH\Lib\\Str"  *)
    (* then the hack server will index the function just as                     *)
    (*   \\Str\\length                                                          *)
    (* The main index, having only a global list if functions/classes, doesn't  *)
    (* actually offer any way for us to iterate over namespaces. And changing   *)
    (* its trie-indexer to do so is kind of ugly. So as a temporary workaround, *)
    (* to give an okay user-experience at least for the Hack standard library,  *)
    (* we're just going to list all the possible standard namespaces right here *)
    (* and see if any of them really exist in the current codebase/hhconfig!    *)
    (* This will give a good experience only for codebases where users rarely   *)
    (* define their own namespaces...                                           *)
    let standard_namespaces =
      ["C"; "Vec"; "Dict"; "Str"; "Keyset"; "Math"; "Regex"; "SecureRandom"; "PHP"; "JS"] in
    let namespace_permutations ns = [
       Printf.sprintf "%s" ns;
       Printf.sprintf "%s\\fb" ns;
       Printf.sprintf "HH\\Lib\\%s" ns;
       Printf.sprintf "HH\\Lib\\%s\\fb" ns;
    ] in
    let all_possible_namespaces =
      List.map standard_namespaces ~f:namespace_permutations |> List.concat
    in
    List.iter all_possible_namespaces ~f:(fun ns ->
      let ns_results = search_funs_and_classes (ns ^ "\\") ~limit:(Some 1)
        ~on_class:(fun _className -> on_namespace ns)
        ~on_function:(fun _functionName -> on_namespace ns)
      in
      List.iter ns_results.With_complete_flag.value add_res
    );

    (* Use search results to look for matches, while excluding names we have
     * already seen in local content buffer *)
    let gname_results = search_funs_and_classes gname ~limit:(Some 100)
      ~on_class:(on_class ~seen:content_classes)
      ~on_function:(on_function ~seen:content_funs)
    in
    autocomplete_is_complete :=
      !autocomplete_is_complete && gname_results.With_complete_flag.is_complete;
    List.iter gname_results.With_complete_flag.value add_res;

    (* Compute global namespace fallback results for functions, if applicable *)
    match gname_gns with
    | Some gname_gns when gname <> gname_gns ->
      let gname_gns_results = search_funs_and_classes gname_gns ~limit:(Some 100)
        ~on_class:(fun _ -> None)
        ~on_function:(on_function ~seen:content_funs)
      in
      autocomplete_is_complete :=
        !autocomplete_is_complete && gname_gns_results.With_complete_flag.is_complete;
      List.iter gname_gns_results.With_complete_flag.value add_res;
    | _ -> ()
  end

(* Here we turn partial_autocomplete_results into complete_autocomplete_results *)
(* by using typing environment to convert ty information into strings. *)
let resolve_ty
    (env: Typing_env.env)
    (autocomplete_context: legacy_autocomplete_context)
    (x: partial_autocomplete_result)
  : complete_autocomplete_result =
  let env, ty = match x.ty with
    | DeclTy ty -> Phase.localize_with_self env ty
    | LoclTy ty -> env, ty
  in
  let desc_string = match x.kind_ with
    | Method_kind
    | Function_kind
    | Variable_kind
    | Property_kind
    | Class_constant_kind
    | Constructor_kind -> Typing_print.full_strip_ns env ty
    | Abstract_class_kind -> "abstract class"
    | Class_kind -> "class"
    | Interface_kind -> "interface"
    | Trait_kind -> "trait"
    | Enum_kind -> "enum"
    | Namespace_kind -> "namespace"
    | Keyword_kind -> "keyword"
  in
  let func_details = match ty with
    | (_, Tfun ft) ->
      let param_to_record ?(is_variadic=false) param =
        {
          param_name     = (match param.fp_name with
                             | Some n -> n
                             | None -> "");
          param_ty       = Typing_print.full_strip_ns env param.fp_type;
          param_variadic = is_variadic;
        }
      in
      Some {
        return_ty = Typing_print.full_strip_ns env ft.ft_ret;
        min_arity = arity_min ft.ft_arity;
        params    = List.map ft.ft_params param_to_record @
          (match ft.ft_arity with
             | Fellipsis _ ->
                 let empty = TUtils.default_fun_param (Reason.none, Tany) in
                 [param_to_record ~is_variadic:true empty]
             | Fvariadic (_, p) -> [param_to_record ~is_variadic:true p]
             | Fstandard _ -> [])
      }
    | _ -> None
  in
  (* XHP class+attribute names are stored internally with a leading colon.    *)
  (* We'll render them without it if and only if we're in an XHP context.     *)
  (*   $x = new :class1() -- do strip the colon in front of :class1           *)
  (*   $x = <:class1      -- don't strip it                                   *)
  (*   $x->:attr1         -- don't strip the colon in front of :attr1         *)
  (*   <class1 :attr="a"  -- do strip it                                      *)
  (* The logic is thorny here because we're relying upon regexes to figure    *)
  (* out the context. Once we switch autocomplete to FFP, it'll be cleaner.   *)
  let name = match x.kind_, autocomplete_context with
    | Property_kind, { AutocompleteTypes.is_instance_member = false; _ } -> lstrip x.name ":"
    | Abstract_class_kind, { AutocompleteTypes.is_xhp_classname = true; _ }
    | Class_kind, { AutocompleteTypes.is_xhp_classname = true; _ } -> lstrip x.name ":"
    | _ -> x.name
  in
  {
    res_pos      = (fst ty) |> Typing_reason.to_pos |> Pos.to_absolute;
    res_ty       = desc_string;
    res_name     = name;
    res_kind     = x.kind_;
    func_details = func_details;
  }


let get_results
    ~tcopt
    ~delimit_on_namespaces
    ~content_funs
    ~content_classes
    ~autocomplete_context
  =
  Errors.ignore_ begin fun () ->
    let completion_type = !argument_global_type in
    if completion_type = Some Acid ||
       completion_type = Some Acnew ||
       completion_type = Some Actype
    then compute_complete_global
      ~tcopt ~delimit_on_namespaces ~autocomplete_context ~content_funs ~content_classes;
    let env = match !ac_env with
      | Some e -> e
      | None -> Typing_env.empty tcopt Relative_path.default ~droot:None
    in
    let resolve (result: autocomplete_result) : complete_autocomplete_result =
      match result with
      | Partial res -> resolve_ty env autocomplete_context res
      | Complete res -> res
    in
    {
      With_complete_flag.is_complete = !autocomplete_is_complete;
      value = !autocomplete_results |> List.map ~f:resolve;
    }
end

let reset () =
  auto_complete_for_global := "";
  argument_global_type := None;
  auto_complete_pos := None;
  ac_env := None;
  autocomplete_results := [];
  autocomplete_is_complete := true

let attach_hooks () =
  reset();
  Autocomplete.auto_complete := true;
  Typing_hooks.attach_id_hook autocomplete_id;
  Typing_hooks.attach_smethod_hook autocomplete_smethod;
  Typing_hooks.attach_cmethod_hook autocomplete_cmethod;
  Typing_hooks.attach_lvar_hook autocomplete_lvar;
  Typing_hooks.attach_new_id_hook autocomplete_new;
  Naming_hooks.attach_hint_hook autocomplete_hint

let detach_hooks () =
  reset();
  Autocomplete.auto_complete := false;
  Typing_hooks.remove_all_hooks();
  Naming_hooks.remove_all_hooks()
