(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Tools for making sha1 digests. *)

open Core_kernel
(**
 * The output from sha1sum always includes a filename, even if the input
 * comes from stdin (then it just prints a dash). Annoying.
 *
 * This changes
 * "5867f5b5093dd9ae86c5a64a3c31ed4c5cba1289  -"
 * to
 * 5867f5b5093dd9ae86c5a64a3c31ed4c5cba1289
 *)
let strip_output_filename digest =
  let expected_digest_length = 40 in
  let open Option in
  let index = String.index digest ' ' in
  index >>| (fun len -> String.sub digest ~pos:0 ~len) >>= fun digest ->
    if (String.length digest) = expected_digest_length then
      Some digest
    else
      None

let digest message =
  let process = Process.exec "sha1sum" ~input:message [] in
  match Process.read_and_wait_pid ~timeout:30 process with
  | Ok {Process_types.stdout; _} ->
    strip_output_filename stdout
  | Error _ ->
    None
