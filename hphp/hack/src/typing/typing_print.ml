(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)


(*****************************************************************************)
(* Pretty printing of types *)
(*****************************************************************************)

open Core_kernel
open Typing_defs
open Typing_logic
open Utils

module SN = Naming_special_names
module Reason = Typing_reason
module TySet = Typing_set

(*****************************************************************************)
(* Computes the string representing a type in an error message.
 * We generally don't want to show the whole type. If an error was due
 * because something is a Vector instead of an int, we don't want to show
 * the type Vector<Vector<array<int>>> because it could be misleading.
 * The error is due to the fact that it is a Vector, regardless of the
 * type parameters.
 *)
(*****************************************************************************)

module ErrorString = struct

  let tprim = function
    | Nast.Tvoid       -> "void"
    | Nast.Tint        -> "an int"
    | Nast.Tbool       -> "a bool"
    | Nast.Tfloat      -> "a float"
    | Nast.Tstring     -> "a string"
    | Nast.Tnum        -> "a num (int/float)"
    | Nast.Tresource   -> "a resource"
    | Nast.Tarraykey   -> "an array key (int/string)"
    | Nast.Tnoreturn   -> "noreturn (throws or exits)"

  let varray = "a varray"
  let darray = "a darray"
  let varray_or_darray = "a varray_or_darray"

  let rec type_: type a. a ty_ -> _ = function
    | Tany               -> "an untyped value"
    | Terr               -> "a type error"
    | Tdynamic           -> "a dynamic value"
    | Tunresolved l      -> unresolved l
    | Tarray (x, y)      -> array (x, y)
    | Tdarray (_, _)     -> darray
    | Tvarray _          -> varray
    | Tarraykind (AKvarray_or_darray _) -> varray_or_darray
    | Tvarray_or_darray _ -> varray_or_darray
    | Tarraykind AKempty -> "an empty array"
    | Tarraykind AKany   -> array (None, None)
    | Tarraykind AKvarray _
                         -> varray
    | Tarraykind (AKvec x)
                         -> array (Some x, None)
    | Tarraykind AKdarray (_, _)
                         -> darray
    | Tarraykind (AKmap (x, y))
                         -> array (Some x, Some y)
    | Tarraykind (AKshape _)
                         -> "an array (used like a shape)"
    | Tarraykind (AKtuple _)
                         -> "an array (used like a tuple)"
    | Ttuple l           -> "a tuple of size " ^ string_of_int (List.length l)
    | Tmixed             -> "a mixed value"
    | Tnonnull           -> "a nonnull value"
    | Toption (_, Tnonnull) -> "a mixed value"
    | Toption _          -> "a nullable type"
    | Tprim tp           -> tprim tp
    | Tvar _             -> "some value"
    | Tanon _    -> "a function"
    | Tfun _     -> "a function"
    | Tgeneric x    -> "a value of declared generic type " ^ x
    | Tabstract (AKnewtype (x, _), _)
        when x = SN.Classes.cClassname -> "a classname string"
    | Tabstract (AKnewtype (x, _), _)
        when x = SN.Classes.cTypename -> "a typename string"
    | Tabstract (ak, cstr) -> abstract ak cstr
    | Tclass ((_, x), _) -> "an object of type "^(strip_ns x)
    | Tapply ((_, x), _)
        when x = SN.Classes.cClassname -> "a classname string"
    | Tapply ((_, x), _)
        when x = SN.Classes.cTypename -> "a typename string"
    | Tapply ((_, x), _) -> "an object of type "^(strip_ns x)
    | Tobject            -> "an object"
    | Tshape _           -> "a shape"
    | Taccess (root_ty, ids) -> tconst root_ty ids
    | Tthis -> "the type 'this'"

  and array: type a. a ty option * a ty option -> _ = function
    | None, None     -> "an untyped array"
    | Some _, None   -> "an array (used like a vector)"
    | Some _, Some _ -> "an array (used like a hashtable)"
    | _              -> assert false

  and abstract ak cstr =
    let x = strip_ns @@ AbstractKind.to_string ak in
    match ak, cstr with
    | AKnewtype (_, _), _ -> "an object of type "^x
    | AKenum _, _ -> "a value of "^x
    | AKgeneric s, _ when AbstractKind.is_generic_dep_ty s ->
      "the expression dependent type "^s
    | AKgeneric _, _ -> "a value of generic type "^x
    | AKdependent (`cls c, []), Some (_, ty) ->
        type_ ty^" (known to be exactly the class '"^strip_ns c^"')"
    | AKdependent ((`static | `expr _), _), _ ->
        "the expression dependent type "^x
    | AKdependent (_, _::_), _ -> "the abstract type constant "^x
    | AKdependent _, _ ->
        "the type '"^x^"'"
        ^Option.value_map cstr ~default:""
          ~f:(fun (_, ty) -> "\n  that is compatible with "^type_ ty)

  and unresolved l =
    let l = List.map l snd in
    let l = List.map l type_ in
    let s = List.fold_right l ~f:SSet.add ~init:SSet.empty in
    let l = SSet.elements s in
    unresolved_ l

  and unresolved_ = function
    | []      -> "an undefined value"
    | [x]     -> x
    | x :: rl -> x^" or "^unresolved_ rl

  and tconst: type a. a ty -> _ -> _ = fun root_ty ids ->
    let f x =
      let x =
        if String.contains x '<'
        then "this"
        else x
      in
      List.fold_left ~f:(fun acc (_, sid) -> acc^"::"^sid)
        ~init:("the type constant "^strip_ns x) ids in
    match snd root_ty with
    | Tgeneric x -> f x
    | Tapply ((_, x), _) -> f x
    | Tclass ((_, x), _) -> f x
    | Tabstract (ak, _) -> f @@ AbstractKind.to_string ak
    | Taccess _ as x ->
        List.fold_left ~f:(fun acc (_, sid) -> acc^"::"^sid)
          ~init:(type_ x) ids
     | _ ->
         "a type constant"

  and class_kind c_kind final =
    let fs = if final then " final" else "" in
    match c_kind with
    | Ast.Cabstract -> "an abstract" ^ fs ^ " class"
    | Ast.Cnormal -> "a" ^ fs ^ " class"
    | Ast.Cinterface -> "an interface"
    | Ast.Ctrait -> "a trait"
    | Ast.Cenum -> "an enum"
end

(*****************************************************************************)
(* Module used to "suggest" types.
 * When a type is missing, it is nice to suggest a type to the user.
 * However, there are some cases where parts of the type is still unresolved.
 * When that is the case, we print '...' and let the user replace the missing
 * parts with a real type. So if we inferred that something was a Vector,
 * but we didn't manage to infer the type of the elements, the output becomes:
 * Vector<...>.
 *)
(*****************************************************************************)

module Suggest = struct

  let rec type_: type a. a ty -> string = fun (_, ty) ->
    match ty with
    | Tarray _               -> "array"
    | Tdarray _              -> "darray"
    | Tvarray _              -> "varray"
    | Tvarray_or_darray _    -> "varray_or_darray"
    | Tarraykind AKdarray (_, _)
                             -> "darray"
    | Tarraykind AKvarray _  -> "varray"
    | Tarraykind _           -> "array"
    | Tdynamic               -> "dynamic"
    | Tthis                  -> SN.Typehints.this
    | Tunresolved _          -> "..."
    | Ttuple (l)             -> "("^list l^")"
    | Tany                   -> "..."
    | Terr                   -> "..."
    | Tmixed                 -> "mixed"
    | Tnonnull               -> "nonnull"
    | Tgeneric s             -> s
    | Tabstract (AKgeneric s, _) -> s
    | Toption (_, Tnonnull)  -> "mixed"
    | Toption ty             -> "?" ^ type_ ty
    | Tprim tp               -> prim tp
    | Tvar _                 -> "..."
    | Tanon _       -> "..."
    | Tfun _ -> "..."
    | Tapply ((_, cid), [])  -> Utils.strip_ns cid
    | Tapply ((_, cid), [x]) -> (Utils.strip_ns cid)^"<"^type_ x^">"
    | Tapply ((_, cid), l)   -> (Utils.strip_ns cid)^"<"^list l^">"
    | Tclass ((_, cid), []) -> Utils.strip_ns cid
    | Tabstract ((AKnewtype (cid, []) | AKenum cid), _) -> Utils.strip_ns cid
    | Tclass ((_, cid), [x]) -> (Utils.strip_ns cid)^"<"^type_ x^">"
    | Tabstract (AKnewtype (cid, [x]), _) ->
        (Utils.strip_ns cid)^"<"^type_ x^">"
    | Tclass ((_, cid), l) -> (Utils.strip_ns cid)^"<"^list l^">"
    | Tabstract (AKnewtype (cid, l), _)   ->
        (Utils.strip_ns cid)^"<"^list l^">"
    | Tabstract (AKdependent (_, _), _) -> "..."
    | Tobject                -> "..."
    | Tshape _               -> "..."
    | Taccess (root_ty, ids) ->
        let x =
          match snd root_ty with
          | Tapply ((_, x), _) -> Some x
          | Tthis -> Some SN.Typehints.this
          | _ -> None in
        (match x with
         | None -> "..."
         | Some x ->
            List.fold_left ids
              ~f:(fun acc (_, sid) -> acc^"::"^sid)
              ~init:(strip_ns x)
        )

  and list: type a. a ty list -> string = function
    | []      -> ""
    | [x]     -> type_ x
    | x :: rl -> type_ x ^ ", "^ list rl

  and prim = function
    | Nast.Tvoid   -> "void"
    | Nast.Tint    -> "int"
    | Nast.Tbool   -> "bool"
    | Nast.Tfloat  -> "float"
    | Nast.Tstring -> "string"
    | Nast.Tnum    -> "num (int/float)"
    | Nast.Tresource -> "resource"
    | Nast.Tarraykey -> "arraykey (int/string)"
    | Nast.Tnoreturn -> "noreturn"

end

(*****************************************************************************)
(* Pretty-printer of the "full" type.                                        *)
(* This is used in server/symbolTypeService and elsewhere                    *)
(* With debug_mode set it is used for hh_show_env                            *)
(*****************************************************************************)

module Full = struct
  module Env = Typing_env

  open Doc

  let format_env = Format_env.{default with line_width = 60}

  let text_strip_ns s = Doc.text (Utils.strip_ns s)

  let (^^) a b = Concat [a; b]

  let debug_mode = ref false
  let show_tvars = ref false

  let comma_sep = Concat [text ","; Space]

  let id x = x

  let list_sep ?(split=true) (s : Doc.t) (f : 'a -> Doc.t) (l : 'a list) : Doc.t =
    let split = if split then Split else Nothing in
    let max_idx = List.length l - 1 in
    let elements = List.mapi l ~f:begin fun idx element ->
      if idx = max_idx
      then f element
      else Concat [f element; s; split]
    end in
    match elements with
    | [] -> Nothing
    | xs -> Nest [split; Concat xs; split]

  let delimited_list sep left_delimiter f l right_delimiter =
    Span [
      text left_delimiter;
      WithRule (Rule.Parental, Concat [
        list_sep sep f l;
        text right_delimiter;
      ]);
    ]

  let list: type c. _ -> (c -> Doc.t) -> c list -> _ -> _ =
    fun ld x y rd -> delimited_list comma_sep ld x y rd

  let shape_map fdm f_field =
    let compare = (fun (k1, _) (k2, _) ->
       compare (Env.get_shape_field_name k1) (Env.get_shape_field_name k2)) in
    let fields = List.sort ~compare (Nast.ShapeMap.elements fdm) in
    List.map fields f_field

  let rec ty: type a. _ -> _ -> _ -> a ty -> Doc.t =
    fun to_doc st env (_, x) -> ty_ to_doc st env x

  and ty_: type a. _ -> _ -> _ -> a ty_ -> Doc.t =
    fun to_doc st env x ->
    let k: type b. b ty -> _ = fun x -> ty to_doc st env x in
    match x with
    | Tany -> text "_"
    | Terr -> text (if !debug_mode then "err" else "_")
    | Tthis -> text SN.Typehints.this
    | Tmixed -> text "mixed"
    | Tdynamic -> text "dynamic"
    | Tnonnull -> text "nonnull"
    | Tdarray (x, y) -> list "darray<" k [x; y] ">"
    | Tvarray x -> list "varray<" k [x] ">"
    | Tvarray_or_darray x -> list "varray_or_darray<" k [x] ">"
    | Tarraykind (AKvarray_or_darray x) -> list "varray_or_darray<" k [x] ">"
    | Tarraykind AKany -> text "array"
    | Tarraykind AKempty -> text "array (empty)"
    | Tarray (None, None) -> text "array"
    | Tarraykind AKvarray x -> list "varray<" k [x] ">"
    | Tarraykind (AKvec x) -> list "array<" k [x] ">"
    | Tarray (Some x, None) -> list "array<" k [x] ">"
    | Tarray (Some x, Some y) -> list "array<" k [x; y] ">"
    | Tarraykind AKdarray (x, y) -> list "darray<" k [x; y] ">"
    | Tarraykind (AKmap (x, y)) -> list "array<" k [x; y] ">"
    | Tarraykind (AKshape fdm) ->
      let f_field (shape_map_key, (_tk, tv)) = Concat [
          to_doc (Env.get_shape_field_name shape_map_key);
          Space;
          text "=>";
          Space;
          k tv
        ] in
      list "shape-like-array(" id (shape_map fdm f_field) ")"
    | Tarraykind (AKtuple fields) ->
      list "tuple-like-array(" k (List.rev (IMap.values fields)) ")"
    | Tarray (None, Some _) -> assert false
    | Tclass ((_, s), []) -> to_doc s
    | Tapply ((_, s), []) -> to_doc s
    | Tgeneric s -> to_doc s
    | Taccess (root_ty, ids) -> Concat [
        k root_ty;
        to_doc (List.fold_left ids
          ~f:(fun acc (_, sid) -> acc ^ "::" ^ sid) ~init:"")
      ]
    | Toption (_, Tnonnull) -> text "mixed"
    | Toption x -> Concat [text "?"; k x]
    | Tprim x -> text @@ prim x
    | Tvar n ->
      if TypecheckerOptions.new_inference (Env.get_tcopt env)
      then text ("#" ^ (string_of_int n))
      else
      let _, n' = Env.get_var env n in
      let prepend =
        if ISet.mem n' st then text "[rec]"
        else
        (* For hh_show_env we further show the type variable number *)
        if !show_tvars then (text ("#" ^ (string_of_int n)))
        else Nothing
      in
      let _, ety = Env.expand_type env (Reason.Rnone, x) in
      let st = ISet.add n' st in
      Concat [prepend; ty to_doc st env ety]
    | Tfun ft -> Concat [
        if ft.ft_abstract then text "abs" ^^ Space else Nothing;
        text "(";
        if ft.ft_is_coroutine then text "coroutine" ^^ Space else Nothing;
        text "function";
        fun_type to_doc st env ft;
        text ")";
        (match ft.ft_ret with
          | (Reason.Rdynamic_yield _, _) -> Space ^^ text "[DynamicYield]"
          | _ -> Nothing)
      ]
    | Tclass ((_, s), tyl) -> to_doc s ^^ list "<" k tyl ">"
    | Tabstract (AKnewtype (s, []), _) -> to_doc s
    | Tabstract (AKnewtype (s, tyl), _) -> to_doc s ^^ list "<" k tyl ">"
    (* This is an "exact" type. Show bound, for generics etc *)
    | Tabstract (AKdependent (`cls _, []), Some ty) ->
      if !debug_mode then Concat [text "exact"; Space; k ty]
      else k ty
    | Tabstract (ak, cstr) ->
      let cstr_info = if !debug_mode then
        match cstr with
        | None -> Nothing
        | Some ty -> Concat [Space; text "as"; Space; k ty]
      else Nothing in
      Concat [to_doc @@ AbstractKind.to_string ak; cstr_info]
    (* Don't strip_ns here! We want the FULL type, including the initial slash.
    *)
    | Tapply ((_, s), tyl) -> to_doc s ^^ list "<" k tyl ">"
    | Ttuple tyl -> list "(" k tyl ")"
    | Tanon (_, id) ->
      begin match Env.get_anonymous env id with
      | Some (Reactive _, true, _, _, _) -> text "[coroutine rx fun]"
      | Some (Nonreactive, true, _, _, _) -> text "[coroutine fun]"
      | Some (Reactive _, false, _, _, _) -> text "[rx fun]"
      | _ -> text "[fun]"
      end
    | Tunresolved [] -> text "[unresolved]"
    | Tunresolved [ty] ->
      if !show_tvars then Concat [text "("; k ty; text ")"] else k ty
    | Tunresolved tyl -> delimited_list (Space ^^ text "|" ^^ Space) "(" k tyl ")"
    | Tobject -> text "object"
    | Tshape (fields_known, fdm) ->
      let optional_shape_fields_enabled =
        not @@
          TypecheckerOptions.experimental_feature_enabled
            (Env.get_options env)
            TypecheckerOptions.experimental_disable_optional_and_unknown_shape_fields in
      let fields =
        let f_field (shape_map_key, { sft_optional; sft_ty }) =
        let key_delim =
          match shape_map_key with Ast.SFlit_str _ -> text "'" | _ -> Nothing
        in
          Concat [
            if optional_shape_fields_enabled && sft_optional then text "?" else Nothing;
            key_delim;
            to_doc (Env.get_shape_field_name shape_map_key);
            key_delim;
            Space;
            text "=>";
            Space;
            k sft_ty;
          ]
        in
        shape_map fdm f_field
      in
      let fields =
        match fields_known with
        | FieldsFullyKnown -> fields
        | FieldsPartiallyKnown _ -> fields @ [text "..."]
      in
      Concat [
        list "shape(" id fields ")";
        match fields_known with
        | FieldsFullyKnown -> Nothing
        | FieldsPartiallyKnown unset_fields ->
          match Nast.ShapeMap.elements unset_fields with
          | [] -> Nothing
          | _ -> Concat [
              text "(unset fields:";
              Space;
              Concat (List.map (Nast.ShapeMap.ordered_keys unset_fields) begin fun k ->
                Concat [to_doc (Env.get_shape_field_name k); Space]
              end);
              text ")"
            ]
      ]

  and prim x =
    match x with
    | Nast.Tvoid   -> "void"
    | Nast.Tint    -> "int"
    | Nast.Tbool   -> "bool"
    | Nast.Tfloat  -> "float"
    | Nast.Tstring -> "string"
    | Nast.Tnum    -> "num"
    | Nast.Tresource -> "resource"
    | Nast.Tarraykey -> "arraykey"
    | Nast.Tnoreturn -> "noreturn"

  and fun_type: type a. _ -> _ -> _ -> a fun_type -> _ =
    fun to_doc st env ft ->
    let params = List.map ft.ft_params (fun_param to_doc st env) in
    let variadic_param =
      match ft.ft_arity with
      | Fstandard _ -> None
      | Fellipsis _ -> Some (text "...")
      | Fvariadic (_, p) ->
        Some (Concat [
          (match p.fp_type with
          | _, Tany -> Nothing
          | _ -> fun_param to_doc st env p
          );
          text "..."
        ])
    in
    let params =
      match variadic_param with
      | None -> params
      | Some variadic_param -> params @ [variadic_param]
    in
    Span [
      (match ft.ft_tparams with
      | [] -> Nothing
      | l -> list "<" (tparam to_doc st env) l ">"
      );
      list "(" id params "):";
      Space;
      ty to_doc st env ft.ft_ret
    ]

  and fun_param: type a. _ -> _ -> _ -> a fun_param -> _ =
    fun to_doc st env { fp_name; fp_type; fp_kind; _ } ->
    Concat [
      (match fp_kind with
      | FPinout -> text "inout" ^^ Space
      | _ -> Nothing
      );
      match fp_name, fp_type with
      | None, _ -> ty to_doc st env fp_type
      | Some param_name, (_, Tany) -> text param_name
      | Some param_name, _ ->
          Concat [ty to_doc st env fp_type; Space; text param_name]
    ]

  and tparam: type a. _ -> _ -> _ -> a Typing_defs.tparam -> _ =
    fun to_doc st env (_, (_, x), cstrl, _) ->
      Concat [text x; list_sep ~split:false Space (tparam_constraint to_doc st env) cstrl]

  and tparam_constraint:
    type a. _ -> _ -> _ -> (Ast.constraint_kind * a ty) -> _ =
    fun to_doc st env (ck, cty) ->
      Concat [
        Space;
        text
          (match ck with
          | Ast.Constraint_as -> "as"
          | Ast.Constraint_super -> "super"
          | Ast.Constraint_eq -> "="
          );
        Space;
        ty to_doc st env cty
      ]

  (* For a given type parameter, construct a list of its constraints *)
  let get_constraints_on_tparam env tparam =
    let lower = Env.get_lower_bounds env tparam in
    let upper = Env.get_upper_bounds env tparam in
    let equ = Env.get_equal_bounds env tparam in
    (* If we have an equality we can ignore the other bounds *)
    if not (TySet.is_empty equ)
    then List.map (TySet.elements equ) (fun ty -> (tparam, Ast.Constraint_eq, ty))
    else
      List.map (TySet.elements lower) (fun ty -> (tparam, Ast.Constraint_super, ty))
      @
      List.map (TySet.elements upper) (fun ty -> (tparam, Ast.Constraint_as, ty))

  let to_string to_doc env x =
    ty to_doc ISet.empty env x
    |> Libhackfmt.format_doc_unbroken format_env
    |> String.strip

  let constraints_for_type to_doc env ty =
    let tparams = SSet.elements (Env.get_tparams env ty) in
    let constraints = List.concat_map tparams (get_constraints_on_tparam env) in
    if List.is_empty constraints
    then None
    else
      Some (Concat [
        text "where";
        Space;
        WithRule (Rule.Parental,
          list_sep comma_sep begin fun (tparam, ck, ty) ->
            Concat [text tparam; tparam_constraint to_doc ISet.empty env (ck, ty)]
          end constraints
        )
      ])

  let to_string_rec env n x =
    ty Doc.text (ISet.add n ISet.empty) env x
    |> Libhackfmt.format_doc_unbroken format_env
    |> String.strip

  let to_string_strip_ns env x =
    to_string text_strip_ns env x

  let to_string_decl tcopt (x: decl ty) =
    let env =
      Typing_env.empty tcopt Relative_path.default
        ~droot:None in
    to_string Doc.text env x

  let to_string_with_identity env x occurrence definition_opt =
    let prefix =
      let open SymbolDefinition in
      let print_mod m = text (string_of_modifier m) ^^ Space in
      match definition_opt with
      | None -> Nothing
      | Some def ->
        begin match def.modifiers with
        | [] -> Nothing
        (* It looks weird if we line break after a single modifier. *)
        | [m] -> print_mod m
        | ms -> Concat (List.map ms print_mod) ^^ SplitWith Cost.Base
        end
    in
    let body =
      let open SymbolOccurrence in
      match occurrence, x with
      | { type_ = Class; name; _ }, _ -> Concat [text "class"; Space; text_strip_ns name]

      | { type_ = Function; name; _ }, (_, Tfun ft)
      | { type_ = Method (_, name); _ }, (_, Tfun ft) ->
        (* Use short names for function types since they display a lot more
           information to the user. *)
         Concat [
           text "function";
           Space;
           text_strip_ns name;
           fun_type text_strip_ns ISet.empty env ft;
         ]

      | { type_ = Property _; name; _ }, _
      | { type_ = ClassConst _; name; _ }, _
      | { type_ = GConst; name; _ }, _ ->
        Concat [
          ty text_strip_ns ISet.empty env x;
          Space;
          text_strip_ns name;
        ]

      | _ -> ty text_strip_ns ISet.empty env x
    in
    let constraints =
      constraints_for_type text_strip_ns env x
      |> Option.value_map ~default:Nothing ~f:(fun x -> Concat [Newline; x])
    in
    Concat [prefix; body; constraints]
    |> Libhackfmt.format_doc format_env
    |> String.strip

end

module Json =
struct

open Hh_json

let prim = function
  | Nast.Tvoid   -> "void"
  | Nast.Tint    -> "int"
  | Nast.Tbool   -> "bool"
  | Nast.Tfloat  -> "float"
  | Nast.Tstring -> "string"
  | Nast.Tnum    -> "num"
  | Nast.Tresource -> "resource"
  | Nast.Tarraykey -> "arraykey"
  | Nast.Tnoreturn -> "noreturn"

let param_mode_to_string = function
   | FPnormal -> "normal"
   | FPref -> "ref"
   | FPinout -> "inout"

let string_to_param_mode = function
  | "normal" -> Some FPnormal
  | "ref" -> Some FPref
  | "inout" -> Some FPinout
  | _ -> None

let rec from_type: type a. Typing_env.env -> a ty -> json =
  function env -> function ty ->
  (* Helpers to construct fields that appear in JSON rendering of type *)
  let kind k = ["kind", JSON_String k] in
  let args tys = ["args", JSON_Array (List.map tys (from_type env))] in
  let typ ty = ["type", from_type env ty] in
  let result ty = ["result", from_type env ty] in
  let obj x = JSON_Object x in
  let name x = ["name", JSON_String x] in
  let optional x = ["optional", JSON_Bool x] in
  let is_array x = ["is_array", JSON_Bool x] in
  let empty x = ["empty", JSON_Bool x] in
  let make_field (k, v) =
    let shape_field_name_to_json shape_field =
      (* TODO: need to update userland tooling? *)
      match shape_field with
      | Ast.SFlit_int (_, s) -> Hh_json.JSON_Number s
      | Ast.SFlit_str (_, s) -> Hh_json.JSON_String s
      | Ast.SFclass_const ((_, s1), (_, s2)) ->
        Hh_json.JSON_Array [
          Hh_json.JSON_String s1;
          Hh_json.JSON_String s2;
        ]
    in
    obj @@
    ["name", (shape_field_name_to_json k)] @
    optional v.sft_optional @
    typ v.sft_ty in
  let fields fl =
    ["fields", JSON_Array (List.map fl make_field)] in
  let shape_like_array_field (k, (_, v)) =
    obj @@
    name (Typing_env.get_shape_field_name k) @
    typ v in
  let shape_like_array_fields fl =
    ["fields", JSON_Array (List.map fl shape_like_array_field)] in
  let path ids =
    ["path", JSON_Array (List.map ids (fun id -> JSON_String id))] in
  let as_type opt_ty =
    match opt_ty with
    | None -> []
    | Some ty -> ["as", from_type env ty] in
  match snd ty with
  | Tvar _ ->
    let _, ty = Typing_env.expand_type env ty in
    from_type env ty
  | Tarray(opt_ty1, opt_ty2) ->
    obj @@ kind "array" @ args (Option.to_list opt_ty1 @ Option.to_list opt_ty2)
  | Tthis ->
    obj @@ kind "this"
  | Ttuple tys ->
    obj @@ kind "tuple" @ is_array false @ args tys
  | Tany | Terr ->
    obj @@ kind "any"
  | Tmixed ->
    obj @@ kind "mixed"
  | Tnonnull ->
    obj @@ kind "nonnull"
  | Tdynamic ->
    obj @@ kind "dynamic"
  | Tgeneric s ->
    obj @@ kind "generic" @ is_array false @ name s
  | Tabstract (AKgeneric s, opt_ty) ->
    obj @@ kind "generic" @ is_array true @ name s @ as_type opt_ty
  | Tabstract (AKenum s, opt_ty) ->
    obj @@ kind "enum" @ name s @ as_type opt_ty
  | Tabstract (AKnewtype (s, tys), opt_ty) ->
    obj @@ kind "newtype" @ name s @ args tys @ as_type opt_ty
  | Tabstract (AKdependent (`cls c, ids), opt_ty) ->
    obj @@ kind "path" @ ["type", obj @@ kind "class" @ name c @ args []]
      @ path ids @ as_type opt_ty
  | Tabstract (AKdependent (`expr _, ids), opt_ty) ->
    obj @@ kind "path" @ ["type", obj @@ kind "expr"]
      @ path ids @ as_type opt_ty
  | Tabstract (AKdependent (`this, ids), opt_ty) ->
    obj @@ kind "path" @ ["type", obj @@ kind "this"]
      @ path ids @ as_type opt_ty
  | Tabstract (AKdependent (`static, ids), opt_ty) ->
    obj @@ kind "path" @ ["type", obj @@ kind "static"]
      @ path ids @ as_type opt_ty
  | Toption (_, Tnonnull) ->
    obj @@ kind "mixed"
  | Toption ty ->
    obj @@ kind "nullable" @ args [ty]
  | Tprim tp ->
    obj @@ kind "primitive" @ name (prim tp)
  | Tapply ((_, cid), tys) ->
    obj @@ kind "class" @ name cid @ args tys
  | Tclass ((_, cid), tys) ->
    obj @@ kind "class" @ name cid @ args tys
  | Tobject ->
    obj @@ kind "object"
  | Tshape (fields_known, fl) ->
    let fields_known =
      match fields_known with
      | FieldsFullyKnown -> true
      | FieldsPartiallyKnown _ ->
        (* TODO: maybe don't drop the partially-known fields? *)
        false
    in
    obj @@
      kind "shape" @
      is_array false @
      ["fields_known", JSON_Bool fields_known] @
      fields (Nast.ShapeMap.elements fl)
  | Tunresolved [ty] ->
    from_type env ty
  | Tunresolved tyl ->
    obj @@ kind "union" @ args tyl
  | Taccess (ty, ids) ->
    obj @@ kind "path" @ typ ty @ path (List.map ids snd)
  | Tfun ft ->
    let fun_kind =
      if ft.ft_is_coroutine
      then kind "coroutine"
      else kind "function" in
    let callconv cc = ["callConvention", JSON_String (param_mode_to_string cc)] in
    let param fp = obj @@ callconv fp.fp_kind @ typ fp.fp_type in
    let params fps = ["params", JSON_Array (List.map fps param)] in
    obj @@ fun_kind @ params ft.ft_params @ result ft.ft_ret
  | Tanon _ ->
    obj @@ kind "anon"
  | Tdarray (ty1, ty2) ->
    obj @@ kind "darray" @ args [ty1; ty2]
  | Tvarray ty ->
    obj @@ kind "varray" @ args [ty]
  | Tvarray_or_darray ty ->
    obj @@ kind "varray_or_darray" @ args [ty]
    | Tarraykind (AKvarray_or_darray ty) ->
    obj @@ kind "varray_or_darray" @ args [ty]
  | Tarraykind AKany ->
    obj @@ kind "array" @ empty false @ args []
  | Tarraykind (AKdarray(ty1, ty2)) ->
    obj @@ kind "darray" @ args [ty1; ty2]
  | Tarraykind (AKvarray ty) ->
    obj @@ kind "varray" @ args [ty]
  | Tarraykind (AKvec ty) ->
    obj @@ kind "array" @ empty false @ args [ty]
  | Tarraykind (AKmap (ty1, ty2)) ->
    obj @@ kind "array" @ empty false @ args [ty1; ty2]
  | Tarraykind (AKtuple fields) ->
    obj @@ kind "tuple" @ is_array true @ args (List.rev (IMap.values fields))
  | Tarraykind (AKshape fl) ->
    obj @@ kind "shape" @ is_array true
      @ shape_like_array_fields (Nast.ShapeMap.elements fl)
  | Tarraykind AKempty ->
    obj @@ kind "array" @ empty true @ args []

type 'a deserialized_result = ('a ty, deserialization_error) result

let wrap_json_accessor f = fun x ->
  match (f x) with
  | Ok value -> Ok value
  | Error access_failure ->
    Error (Deserialization_error
      (Hh_json.Access.access_failure_to_string access_failure))

let get_string x = wrap_json_accessor (Hh_json.Access.get_string x)
let get_bool x = wrap_json_accessor (Hh_json.Access.get_bool x)
let get_array x = wrap_json_accessor (Hh_json.Access.get_array x)
let get_val x = wrap_json_accessor (Hh_json.Access.get_val x)
let get_obj x = wrap_json_accessor (Hh_json.Access.get_obj x)
let deserialization_error ~message ~keytrace =
  Error (Deserialization_error
    (message ^ (Hh_json.Access.keytrace_to_string keytrace)))
let not_supported ~message ~keytrace =
  Error (Not_supported
    (message ^ (Hh_json.Access.keytrace_to_string keytrace)))
let wrong_phase ~message ~keytrace =
  Error (Wrong_phase
    (message ^ (Hh_json.Access.keytrace_to_string keytrace)))

let to_locl_ty
  (tcopt: TypecheckerOptions.t)
  ?(keytrace = [])
  (json: Hh_json.json)
  : locl deserialized_result =
  let reason = Reason.none in
  let ty (ty: locl ty_): locl deserialized_result =
    Ok (reason, ty)
  in

  let rec aux
    (json: Hh_json.json)
    ~(keytrace: Hh_json.Access.keytrace)
    : locl deserialized_result =
    let open Result.Monad_infix in
    get_string "kind" (json, keytrace) >>= fun (kind, kind_keytrace) ->
    match kind with
    | "this"->
      not_supported
        ~message:"Cannot deserialize 'this' type."
        ~keytrace

    | "any" ->
      ty Tany
    | "nonnull" ->
      ty Tnonnull
    | "dynamic" ->
      ty Tdynamic

    | "generic" ->
      get_string "name" (json, keytrace) >>= fun (name, _name_keytrace) ->
      get_bool "is_array" (json, keytrace)
        >>= fun (is_array, _is_array_keytrace) ->

      if is_array then
        aux_as json ~keytrace >>= fun as_opt ->
        ty (Tabstract ((AKgeneric name), as_opt))
      else
        wrong_phase
          ~message:"Tgeneric is a decl-phase type."
          ~keytrace

    | "enum" ->
      get_string "name" (json, keytrace) >>= fun (name, _name_keytrace) ->
      aux_as json ~keytrace >>= fun as_opt ->
      ty (Tabstract (AKenum name, as_opt))

    | "newtype" ->
      get_string "name" (json, keytrace) >>= fun (name, name_keytrace) ->
      begin match Typing_lazy_heap.get_typedef tcopt name with
      | Some _typedef ->
        (* We end up only needing the name of the typedef. *)
        Ok name
      | None ->
        if name = "HackSuggest"
        then
          not_supported
            ~message:"HackSuggest types for lambdas are not supported"
            ~keytrace
        else
          deserialization_error
            ~message:("Unknown newtype: " ^ name)
            ~keytrace:name_keytrace
      end >>= fun typedef_name ->

      get_array "args" (json, keytrace) >>= fun (args, args_keytrace) ->
      aux_args args ~keytrace:args_keytrace >>= fun args ->
      aux_as json ~keytrace >>= fun as_opt ->
      ty (Tabstract (AKnewtype (typedef_name, args), as_opt))

    | "path" ->
      get_obj "type" (json, keytrace) >>= fun (type_json, type_keytrace) ->
      get_string "kind" (type_json, type_keytrace) >>=
      fun (path_kind, path_kind_keytrace) ->

      get_array "path" (json, keytrace) >>= fun (ids_array, ids_keytrace) ->
      let ids = map_array
        ids_array
        ~keytrace:ids_keytrace
        ~f:(fun id_str ~keytrace ->
          match id_str with
          | JSON_String id ->
            Ok id
          | _ ->
            deserialization_error
              ~message:"Expected a string"
              ~keytrace
        ) in
      ids >>= fun ids ->

      begin match path_kind with
      | "class" ->
        get_string "name" (type_json, type_keytrace)
          >>= fun (class_name, _class_name_keytrace) ->
        aux_as json ~keytrace >>= fun as_opt ->
        ty (Tabstract (AKdependent (`cls class_name, ids), as_opt))

      | "expr" ->
        not_supported
          ~message:"Cannot deserialize path-dependent type involving an expression"
          ~keytrace

      | "this" ->
        aux_as json ~keytrace >>= fun as_opt ->
        ty (Tabstract (AKdependent (`this, ids), as_opt))

      | "static" ->
        aux_as json ~keytrace >>= fun as_opt ->
        ty (Tabstract (AKdependent (`static, ids), as_opt))

      | path_kind ->
        deserialization_error
          ~message:("Unknown path kind: " ^ path_kind)
          ~keytrace:path_kind_keytrace
      end

    | "darray" ->
      get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
      begin match args with
      | [ty1; ty2] ->
        aux ty1 ~keytrace:("0" :: keytrace) >>= fun ty1 ->
        aux ty2 ~keytrace:("1" :: keytrace) >>= fun ty2 ->
        ty (Tarraykind (AKdarray (ty1, ty2)))

      | _ ->
        deserialization_error
          ~message:(Printf.sprintf
            "Invalid number of type arguments to darray (expected 2): %d"
            (List.length args))
          ~keytrace
      end

    | "varray" ->
      get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
      begin match args with
      | [ty1] ->
        aux ty1 ~keytrace:("0" :: keytrace) >>= fun ty1 ->
        ty (Tarraykind (AKvarray ty1))
      | _ ->
        deserialization_error
          ~message:(Printf.sprintf
            "Invalid number of type arguments to varray (expected 1): %d"
            (List.length args))
          ~keytrace
      end

    | "varray_or_darray" ->
      get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
      begin match args with
      | [ty1] ->
        aux ty1 ~keytrace:("0" :: keytrace) >>= fun ty1 ->
        ty (Tarraykind (AKvarray_or_darray ty1))

      | _ ->
        deserialization_error
          ~message:(Printf.sprintf
            "Invalid number of type arguments to varray_or_darray (expected 1): %d"
            (List.length args))
          ~keytrace
      end

    | "array" ->
      get_bool "empty" (json, keytrace) >>= fun (empty, _empty_keytrace) ->
      get_array "args" (json, keytrace) >>= fun (args, _args_keytrace) ->
      begin match args with
      | [] ->
        if empty
        then ty (Tarraykind AKempty)
        else ty (Tarraykind AKany)

      | [ty1] ->
        aux ty1 ~keytrace:("0" :: keytrace) >>= fun ty1 ->
        ty (Tarraykind (AKvec ty1))

      | [ty1; ty2] ->
        aux ty1 ~keytrace:("0" :: keytrace) >>= fun ty1 ->
        aux ty2 ~keytrace:("1" :: keytrace) >>= fun ty2 ->
        ty (Tarraykind (AKmap (ty1, ty2)))

      | _ ->
        deserialization_error
          ~message:(Printf.sprintf
            "Invalid number of type arguments to array (expected 0-2): %d"
            (List.length args))
          ~keytrace
      end

    | "tuple" ->
      get_bool "is_array" (json, keytrace)
        >>= fun (is_array, _is_array_keytrace) ->
      get_array "args" (json, keytrace) >>= fun (args, args_keytrace) ->
      aux_args args ~keytrace:args_keytrace >>= fun args ->
      if is_array then
        let args =
          List.foldi
            (List.rev args)
            ~init:IMap.empty
            ~f:(fun k acc v -> IMap.add k v acc)
        in
        ty (Tarraykind (AKtuple args))
      else
        ty (Ttuple args)

    | "nullable" ->
      get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
      begin match args with
      | [nullable_ty] ->
        aux nullable_ty ~keytrace:("0" :: keytrace) >>= fun nullable_ty ->
        ty (Toption nullable_ty)
      | _ ->
        deserialization_error
          ~message:(Printf.sprintf
            "Unsupported number of args for nullable type: %d"
            (List.length args))
          ~keytrace
      end

    | "primitive" ->
      get_string "name" (json, keytrace) >>= fun (name, keytrace) ->
      begin match name with
      | "void" -> Ok Nast.Tvoid
      | "int" -> Ok Nast.Tint
      | "bool" -> Ok Nast.Tbool
      | "float" -> Ok Nast.Tfloat
      | "string" -> Ok Nast.Tstring
      | "resource" -> Ok Nast.Tresource
      | "num" -> Ok Nast.Tnum
      | "arraykey" -> Ok Nast.Tarraykey
      | "noreturn" -> Ok Nast.Tnoreturn
      | _ ->
        deserialization_error
          ~message:("Unknown primitive type: " ^ name)
          ~keytrace
      end >>= fun prim_ty ->
      ty (Tprim prim_ty)

    | "class" ->
      get_string "name" (json, keytrace) >>= fun (name, _name_keytrace) ->
      let class_pos =
        match Typing_lazy_heap.get_class tcopt name with
        | Some class_ty ->
          class_ty.tc_pos
        | None ->
          (* Class may not exist (such as in non-strict modes). *)
          Pos.none
      in

      get_array "args" (json, keytrace) >>= fun (args, _args_keytrace) ->
      aux_args args ~keytrace >>= fun tyl ->

      (* NB: "class" could have come from either a `Tapply` or a `Tclass`. Right
      now, we always return a `Tclass`. *)
      ty (Tclass ((class_pos, name), tyl))

    | "object" ->
      ty Tobject

    | "shape" ->
      get_array "fields" (json, keytrace) >>= fun (fields, fields_keytrace) ->
      get_bool "is_array" (json, keytrace)
        >>= fun (is_array, _is_array_keytrace) ->

      let unserialize_field
        field_json
        ~keytrace
        : (
          (Ast_defs.shape_field_name * locl Typing_defs.shape_field_type),
          deserialization_error
        ) result =
        get_val "name" (field_json, keytrace)
          >>= fun (name, name_keytrace) ->

        (* We don't need position information for shape field names. They're
        only used for error messages and the like. *)
        let dummy_pos = Pos.none in
        begin match name with
        | Hh_json.JSON_Number name ->
          Ok (Ast.SFlit_int (dummy_pos, name))
        | Hh_json.JSON_String name ->
          Ok (Ast.SFlit_str (dummy_pos, name))
        | Hh_json.JSON_Array [
            Hh_json.JSON_String name1;
            Hh_json.JSON_String name2;
          ] ->
          Ok (Ast.SFclass_const ((dummy_pos, name1), (dummy_pos, name2)))
        | _ ->
          deserialization_error
            ~message:"Unexpected format for shape field name"
            ~keytrace:name_keytrace
        end >>= fun shape_field_name ->

        (* Optional field may be absent for shape-like arrays. *)
        begin match get_val "optional" (field_json, keytrace) with
        | Ok _ ->
          get_bool "optional" (field_json, keytrace)
            >>| fun (optional, _optional_keytrace) ->
          optional
        | Error _ -> Ok false
        end >>= fun optional ->

        get_obj "type" (field_json, keytrace)
          >>= fun (shape_type, shape_type_keytrace) ->
        aux shape_type ~keytrace:shape_type_keytrace >>= fun shape_field_type ->
        let shape_field_type = {
          sft_optional = optional;
          sft_ty = shape_field_type;
        } in
        Ok (shape_field_name, shape_field_type)
      in
      map_array fields ~keytrace:fields_keytrace ~f:unserialize_field
        >>= fun fields ->

      if is_array then
        (* We don't have enough information to perfectly reconstruct shape-like
        arrays. We're missing the keys in the shape map of the shape fields. *)
        not_supported
          ~message:"Cannot deserialize shape-like array type"
          ~keytrace
      else
        get_bool "fields_known" (json, keytrace)
          >>= fun (fields_known, _fields_known_keytrace) ->
        let fields_known =
          if fields_known
          then FieldsFullyKnown
          else FieldsPartiallyKnown Nast.ShapeMap.empty
        in
        let fields = List.fold
          fields
          ~init:Nast.ShapeMap.empty
          ~f:(fun shape_map (k, v) ->
            Nast.ShapeMap.add k v shape_map
          )
        in
        ty (Tshape (fields_known, fields))

    | "union" ->
      get_array "args" (json, keytrace) >>= fun (args, keytrace) ->
      aux_args args ~keytrace >>= fun tyl ->
      ty (Tunresolved tyl)

    | "function"
    | "coroutine" as kind ->
      let ft_is_coroutine = (kind = "coroutine") in
      get_array "params" (json, keytrace) >>= fun (params, params_keytrace) ->
      let params = map_array
        params
        ~keytrace:params_keytrace
        ~f:(fun param ~keytrace ->
          get_string "callConvention" (param, keytrace)
            >>= fun (callconv, callconv_keytrace) ->
          begin match (string_to_param_mode callconv) with
          | Some callconv ->
            Ok callconv
          | None ->
            deserialization_error
              ~message:("Unknown calling convention: " ^ callconv)
              ~keytrace:callconv_keytrace
          end >>= fun callconv ->

          get_obj "type" (param, keytrace)
            >>= fun (param_type, param_type_keytrace) ->
          aux param_type ~keytrace:param_type_keytrace
            >>= fun param_type ->
          Ok {
            fp_type = param_type;
            fp_kind = callconv;

            (* Dummy values: these aren't currently serialized. *)
            fp_pos = Pos.none;
            fp_name = None;
            fp_accept_disposable = false;
            fp_mutability = None;
            fp_rx_annotation = None;
          }
        )
      in
      params >>= fun ft_params ->

      get_obj "result" (json, keytrace) >>= fun (result, result_keytrace) ->
      aux result ~keytrace:result_keytrace >>= fun ft_ret ->
      ty (Tfun {
        ft_is_coroutine;
        ft_params;
        ft_ret;

        (* Dummy values: these aren't currently serialized. *)
        ft_pos = Pos.none;
        ft_deprecated = None;
        ft_abstract = false;
        ft_arity = Fstandard (0, 0);
        ft_tparams = [];
        ft_where_constraints = [];
        ft_ret_by_ref = false;
        ft_reactive = Nonreactive;
        ft_return_disposable = false;
        ft_mutability = None;
        ft_returns_mutable = false;
        ft_decl_errors = None;
        ft_returns_void_to_rx = false;
      })

    | "anon" ->
      not_supported
        ~message:"Cannot deserialize lambda expression type"
        ~keytrace

    | _ ->
      deserialization_error
        ~message:(Printf.sprintf
          "Unknown or unsupported kind '%s' to convert to locl phase"
          kind)
        ~keytrace:kind_keytrace

  and map_array:
    type a.
    Hh_json.json list ->
    f:
      (Hh_json.json ->
      keytrace: Hh_json.Access.keytrace ->
      (a, deserialization_error) result) ->
    keytrace: Hh_json.Access.keytrace ->
    (a list, deserialization_error) result =
    fun array ~f ~keytrace ->
    let array = List.mapi array ~f:(fun i elem ->
      f elem ~keytrace:((string_of_int i) :: keytrace)
    ) in
    Result.all array

  and aux_args
    (args: Hh_json.json list)
    ~(keytrace: Hh_json.Access.keytrace)
    : (locl ty list, deserialization_error) result =
    map_array args ~keytrace ~f:aux

  and aux_as
    (json: Hh_json.json)
    ~(keytrace: Hh_json.Access.keytrace)
    : (locl ty option, deserialization_error) result =
    let open Result.Monad_infix in
    (* as-constraint is optional, check to see if it exists. *)
    match Hh_json.Access.get_obj "as" (json, keytrace) with
    | Ok (as_json, as_keytrace) ->
      aux as_json ~keytrace:as_keytrace >>= fun as_ty ->
      Ok (Some as_ty)
    | Error (Hh_json.Access.Missing_key_error _) ->
      Ok None
    | Error access_failure ->
      deserialization_error
        ~message:("Invalid as-constraint: "
          ^ Hh_json.Access.access_failure_to_string access_failure)
        ~keytrace
  in

  aux json ~keytrace

end

let to_json = Json.from_type
let json_to_locl_ty = Json.to_locl_ty

(*****************************************************************************)
(* Prints the internal type of a class, this code is meant to be used for
 * debugging purposes only.
 *)
(*****************************************************************************)

module PrintClass = struct

  let indent = "    "
  let bool = string_of_bool
  let sset s =
    let contents = SSet.fold (fun x acc -> x^" "^acc) s "" in
    Printf.sprintf "Set( %s)" contents

  let pos p =
    let line, start, end_ = Pos.info_pos p in
    Printf.sprintf "(line %d: chars %d-%d)" line start end_

  let class_kind = function
    | Ast.Cabstract -> "Cabstract"
    | Ast.Cnormal -> "Cnormal"
    | Ast.Cinterface -> "Cinterface"
    | Ast.Ctrait -> "Ctrait"
    | Ast.Cenum -> "Cenum"

  let constraint_ty tcopt = function
    | (Ast.Constraint_as, ty) -> "as " ^ (Full.to_string_decl tcopt ty)
    | (Ast.Constraint_eq, ty) -> "= " ^ (Full.to_string_decl tcopt ty)
    | (Ast.Constraint_super, ty) -> "super " ^ (Full.to_string_decl tcopt ty)

  let variance = function
    | Ast.Covariant -> "+"
    | Ast.Contravariant -> "-"
    | Ast.Invariant -> ""

  let tparam tcopt (var, (position, name), cstrl, reified) =
    variance var^pos position^" "^name^" "^
    (List.fold_right
      cstrl
      ~f:(fun x acc -> constraint_ty tcopt x^" "^acc)
      ~init:"")
    ^ (if reified then " reified" else "")

  let tparam_list tcopt l =
    List.fold_right l ~f:(fun x acc -> tparam tcopt x^", "^acc) ~init:""

  let class_elt tcopt { ce_visibility; ce_synthesized; ce_type = lazy ty; _ } =
    let vis =
      match ce_visibility with
      | Vpublic -> "public"
      | Vprivate _ -> "private"
      | Vprotected _ -> "protected"
    in
    let synth = (if ce_synthesized then "synthetic " else "") in
    let type_ = Full.to_string_decl tcopt ty in
    synth^vis^" "^type_

  let class_elt_smap tcopt m =
    SMap.fold begin fun field v acc ->
      "("^field^": "^class_elt tcopt v^") "^acc
    end m ""

  let class_elt_smap_with_breaks tcopt m =
    SMap.fold begin fun field v acc ->
      "\n"^indent^field^": "^(class_elt tcopt v)^acc
    end m ""

  let class_const_smap tcopt m =
    SMap.fold begin fun field cc acc ->
      let synth = if cc.cc_synthesized then "synthetic " else "" in
      "("^field^": "^synth^Full.to_string_decl tcopt cc.cc_type^") "^acc
    end m ""

  let typeconst tcopt {
    ttc_name = tc_name;
    ttc_constraint = tc_constraint;
    ttc_type = tc_type;
    ttc_origin = origin;
  } =
    let name = snd tc_name in
    let ty x = Full.to_string_decl tcopt x in
    let constraint_ =
      match tc_constraint with
      | None -> ""
      | Some x -> " as "^ty x
    in
    let type_ =
      match tc_type with
      | None -> ""
      | Some x -> " = "^ty x
    in
    name^constraint_^type_^" (origin:"^origin^")"

  let typeconst_smap tcopt m =
    SMap.fold begin fun _ v acc ->
      "\n("^(typeconst tcopt v)^")"^acc
    end m ""

  let ancestors_smap tcopt m =
    (* Format is as follows:
     *    ParentKnownToHack
     *  ! ParentCompletelyUnknown
     *  ~ ParentPartiallyKnown  (interface|abstract|trait)
     *
     * ParentPartiallyKnown must inherit one of the ! Unknown parents, so that
     * sigil could be omitted *)
    SMap.fold begin fun field v acc ->
      let sigil, kind = match Typing_lazy_heap.get_class tcopt field with
        | None -> "!", ""
        | Some {tc_members_fully_known; tc_kind; _} ->
          (if tc_members_fully_known then " " else "~"),
          " ("^class_kind tc_kind^")"
      in
      let ty_str = Full.to_string_decl tcopt v in
      "\n"^indent^sigil^" "^ty_str^kind^acc
    end m ""

  let constructor tcopt (ce_opt, consist) =
    let consist_str = if consist then " (consistent in hierarchy)" else "" in
    let ce_str = match ce_opt with
      | None -> ""
      | Some ce -> class_elt tcopt ce
    in ce_str^consist_str

  let req_ancestors tcopt xs =
    List.fold_left xs ~init:"" ~f:begin fun acc (_p, x) ->
      acc ^ Full.to_string_decl tcopt x ^ ", "
    end

  let class_type tcopt c =
    let tc_need_init = bool c.tc_need_init in
    let tc_members_fully_known = bool c.tc_members_fully_known in
    let tc_abstract = bool c.tc_abstract in
    let tc_deferred_init_members = sset c.tc_deferred_init_members in
    let tc_kind = class_kind c.tc_kind in
    let tc_name = c.tc_name in
    let tc_tparams = tparam_list tcopt c.tc_tparams in
    let tc_consts = class_const_smap tcopt c.tc_consts in
    let tc_typeconsts = typeconst_smap tcopt c.tc_typeconsts in
    let tc_props = class_elt_smap tcopt c.tc_props in
    let tc_sprops = class_elt_smap tcopt c.tc_sprops in
    let tc_methods = class_elt_smap_with_breaks tcopt c.tc_methods in
    let tc_smethods = class_elt_smap_with_breaks tcopt c.tc_smethods in
    let tc_construct = constructor tcopt c.tc_construct in
    let tc_ancestors = ancestors_smap tcopt c.tc_ancestors in
    let tc_req_ancestors = req_ancestors tcopt c.tc_req_ancestors in
    let tc_req_ancestors_extends = sset c.tc_req_ancestors_extends in
    let tc_extends = sset c.tc_extends in
    "tc_need_init: "^tc_need_init^"\n"^
    "tc_members_fully_known: "^tc_members_fully_known^"\n"^
    "tc_abstract: "^tc_abstract^"\n"^
    "tc_deferred_init_members: "^tc_deferred_init_members^"\n"^
    "tc_kind: "^tc_kind^"\n"^
    "tc_name: "^tc_name^"\n"^
    "tc_tparams: "^tc_tparams^"\n"^
    "tc_consts: "^tc_consts^"\n"^
    "tc_typeconsts: "^tc_typeconsts^"\n"^
    "tc_props: "^tc_props^"\n"^
    "tc_sprops: "^tc_sprops^"\n"^
    "tc_methods: "^tc_methods^"\n"^
    "tc_smethods: "^tc_smethods^"\n"^
    "tc_construct: "^tc_construct^"\n"^
    "tc_ancestors: "^tc_ancestors^"\n"^
    "tc_extends: "^tc_extends^"\n"^
    "tc_req_ancestors: "^tc_req_ancestors^"\n"^
    "tc_req_ancestors_extends: "^tc_req_ancestors_extends^"\n"^
    ""
end

module PrintFun = struct

  let fparam tcopt { fp_name = sopt; fp_type = ty; _ } =
    let s = match sopt with
      | None -> "[None]"
      | Some s -> s in
    s ^ " " ^ (Full.to_string_decl tcopt ty) ^ ", "

  let farity = function
    | Fstandard (min, max) -> Printf.sprintf "non-variadic: %d to %d" min max
    | Fvariadic (min, _) ->
      Printf.sprintf "variadic: ...$arg-style (PHP 5.6); min: %d" min
    | Fellipsis (min, _) -> Printf.sprintf "variadic: ...-style (Hack); min: %d" min

  let fparams tcopt l =
    List.fold_right l ~f:(fun x acc -> (fparam tcopt x)^acc) ~init:""

  let fun_type tcopt f =
    let ft_pos = PrintClass.pos f.ft_pos in
    let ft_abstract = string_of_bool f.ft_abstract in
    let ft_arity = farity f.ft_arity in
    let ft_tparams = PrintClass.tparam_list tcopt f.ft_tparams in
    let ft_params = fparams tcopt f.ft_params in
    let ft_ret = Full.to_string_decl tcopt f.ft_ret in
    "ft_pos: "^ft_pos^"\n"^
    "ft_abstract: "^ft_abstract^"\n"^
    "ft_arity: "^ft_arity^"\n"^
    "ft_tparams: "^ft_tparams^"\n"^
    "ft_params: "^ft_params^"\n"^
    "ft_ret: "^ft_ret^"\n"^
    ""
end

module PrintTypedef = struct

  let typedef tcopt = function
    | {td_pos; td_vis = _; td_tparams; td_constraint; td_type;
        td_decl_errors = _;} ->
      let tparaml_s = PrintClass.tparam_list tcopt td_tparams in
      let constr_s = match td_constraint with
        | None -> "[None]"
        | Some constr -> Full.to_string_decl tcopt constr in
      let ty_s = Full.to_string_decl tcopt td_type in
      let pos_s = PrintClass.pos td_pos in
      "ty: "^ty_s^"\n"^
      "tparaml: "^tparaml_s^"\n"^
      "constraint: "^constr_s^"\n"^
      "pos: "^pos_s^"\n"^
      ""

end

(*****************************************************************************)
(* User API *)
(*****************************************************************************)

let error: type a. a ty_ -> _ = fun ty -> ErrorString.type_ ty
let suggest: type a. a ty -> _ =  fun ty -> Suggest.type_ ty
let full env ty = Full.to_string Doc.text env ty
let full_rec env n ty = Full.to_string_rec env n ty
let full_strip_ns env ty = Full.to_string_strip_ns env ty
let full_with_identity = Full.to_string_with_identity
let debug env ty =
  Full.debug_mode := true;
  let f_str = full_strip_ns env ty in
  Full.debug_mode := false;
  f_str

let debug_with_tvars env ty =
  Full.show_tvars := true;
  let f_str = debug env ty in
  Full.show_tvars := false;
  f_str

let class_ tcopt c = PrintClass.class_type tcopt c
let gconst tcopt gc = Full.to_string_decl tcopt (fst gc)
let fun_ tcopt f = PrintFun.fun_type tcopt f
let typedef tcopt td = PrintTypedef.typedef tcopt td
let constraints_for_type env ty =
  Full.constraints_for_type Doc.text env ty
  |> Option.map ~f:(Libhackfmt.format_doc_unbroken Full.format_env)
  |> Option.map ~f:String.strip
let class_kind c_kind final = ErrorString.class_kind c_kind final
let rec subtype_prop env = function
  | Unsat _ -> "UNSAT"
  | Conj [] -> "TRUE"
  | Conj ps ->
    "(" ^ (String.concat ~sep:" && " (List.map ~f:(subtype_prop env) ps)) ^ ")"
  | Disj [] -> "FALSE"
  | Disj ps ->
    "(" ^ (String.concat ~sep:" || " (List.map ~f:(subtype_prop env) ps)) ^ ")"
  | Sub (ty1, ty2) ->
    debug_with_tvars env ty1 ^ " <: " ^ debug_with_tvars env ty2
  | Eq (ty1, ty2) ->
    debug_with_tvars env ty1 ^ " = " ^ debug_with_tvars env ty2
