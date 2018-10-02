module Types = struct

  (** Commandline arg "--with-mini-state" constructs this record. *)
  type mini_state_target_info = {
    changes: Relative_path.t list;
    corresponding_base_revision: string;
    deptable_fn: string;
    prechecked_changes: Relative_path.t list;
    saved_state_fn: string;
  }

  type mini_state_target =
    | Informant_induced_mini_state_target of ServerMonitorUtils.target_mini_state
    | Mini_state_target_info of mini_state_target_info

end

module Abstract_types = struct
  type options
end

module type S = sig
  include module type of Types
  include module type of Abstract_types

  (****************************************************************************)
  (* The main entry point *)
  (****************************************************************************)

  val default_options: root:string -> options
  val parse_options: unit -> options
  val print_json_version: unit -> unit

  (****************************************************************************)
  (* Accessors *)
  (****************************************************************************)

  val ai_mode: options -> Ai_options.t option
  val check_mode: options -> bool
  val config: options -> (string * string) list
  val convert: options -> Path.t option
  val dynamic_view: options -> bool
  val file_info_on_disk: options -> bool
  val from: options -> string
  val gen_saved_ignore_type_errors: options -> bool
  val ignore_hh_version: options -> bool
  val json_mode: options -> bool
  val load_state_canary: options -> bool
  val max_procs: options -> int
  val no_load: options -> bool
  val prechecked: options -> bool option
  val profile_log: options -> bool
  val root: options -> Path.t
  val save_filename: options -> string option
  val should_detach: options -> bool
  val waiting_client: options -> Unix.file_descr option
  val watchman_debug_logging: options -> bool
  val with_mini_state: options -> mini_state_target option

  (****************************************************************************)
  (* Setters *)
  (****************************************************************************)

  val set_gen_saved_ignore_type_errors: options -> bool -> options
  val set_mini_state_target: options -> ServerMonitorUtils.target_mini_state option -> options
  val set_no_load: options -> bool -> options

  (****************************************************************************)
  (* Misc *)
  (****************************************************************************)
  val to_string: options -> string
end
