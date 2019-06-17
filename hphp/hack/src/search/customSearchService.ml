(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *)

(* Put any initialization code necessary here *)
let initialize (): unit =
  ()

(* If you have a way of listing namespaces, put it here *)
let fetch_namespaces (): string list =
  []

(* Use the custom search service to find symbols by autocomplete context *)
let search_symbols
    ~(query_text: string)
    ~(max_results: int)
    ~(context: SearchUtils.autocomplete_type option)
  : SearchUtils.si_results =
  let _ = query_text in
  let _ = max_results in
  let _ = context in
  []
