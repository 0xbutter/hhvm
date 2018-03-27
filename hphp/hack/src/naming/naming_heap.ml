(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* Mapping the canonical name (lower case form) to the actual name *)
module type CanonHeap =
  SharedMem.NoCache with type t = string
               and type key = string
               and module KeySet = Set.Make (StringKey)

module TypeCanonHeap : CanonHeap = SharedMem.NoCache (StringKey) (struct
  type t = string
  let prefix = Prefix.make()
  let description = "TypeCanon"
  let use_sqlite_fallback () = false
end)

module FunCanonHeap : CanonHeap = SharedMem.NoCache (StringKey) (struct
  type t = string
  let prefix = Prefix.make()
  let description = "FunCanon"
  let use_sqlite_fallback () = false
end)

let check_valid key pos =
  if FileInfo.get_pos_filename pos = Relative_path.default then begin
    Hh_logger.log
      ("WARNING: setting canonical position of %s to be in dummy file. If this \
  happens in incremental mode, things will likely break later.") key;
    Hh_logger.log "%s"
      (Printexc.raw_backtrace_to_string (Printexc.get_callstack 100));
  end

(* TypeIdHeap records both class names and typedefs since they live in the
 * same namespace. That is, one cannot both define a class Foo and a typedef
 * Foo (or FOO or fOo, due to case insensitivity). *)
module TypeIdHeap = struct
  include SharedMem.WithCache (StringKey) (struct
    type t = FileInfo.pos * [`Class | `Typedef]
    let prefix = Prefix.make ()
    let description = "TypeId"
    let use_sqlite_fallback () = false
  end)
  let add x y =
    if not @@ LocalChanges.has_local_changes () then check_valid x (fst y);
    add x y

  let write_through x y =
    if not @@ LocalChanges.has_local_changes () then check_valid x (fst y);
    write_through x y
end

module FunPosHeap = struct
  include SharedMem.NoCache (StringKey) (struct
    type t = FileInfo.pos
    let prefix = Prefix.make()
    let description = "FunPos"
    let use_sqlite_fallback () = false
  end)
  let add x y =
    if not @@  LocalChanges.has_local_changes () then check_valid x y;
    add x y
end

module ConstPosHeap = struct
  include SharedMem.NoCache (StringKey) (struct
    type t = FileInfo.pos
    let prefix = Prefix.make()
    let description = "ConstPos"
    let use_sqlite_fallback () = false
  end)
let add x y =
  if not @@ LocalChanges.has_local_changes () then check_valid x y;
  add x y
end
