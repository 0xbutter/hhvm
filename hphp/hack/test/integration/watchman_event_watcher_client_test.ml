module Args = Test_harness_common_args
module WEW = WatchmanEventWatcher
module WEWClient = WatchmanEventWatcherClient
module WEWConfig = WatchmanEventWatcherConfig

module BA = Asserter.Bool_asserter

exception Process_failed
exception Init_watcher_client_failed
exception Retries_exhausted of string

let assert_process_completes_ok ?(timeout=30) ~msg p =
  match Process.read_and_wait_pid ~timeout p with
  | Result.Ok result ->
    result
  | Result.Error f -> begin
    Printf.eprintf "%s failed because:\n%s\n" msg (Process.failure_msg f);
    raise Process_failed
  end

let rec init_watcher_client ~retries repo =
  if retries < 0 then
    raise Init_watcher_client_failed
  else
    let client = WEWClient.init repo in
    match client with
    | Some client ->
      client
    | None ->
      let () = Sys_utils.sleep 1.0 in
      init_watcher_client ~retries:(retries - 1) repo

let create_watcher_socket_file repo =
  let socket_file = WEWConfig.socket_file repo in
  Sys_utils.mkdir_p (Filename.dirname socket_file);
  Sys_utils.write_file ~file:socket_file ""

let hg_commit ?(add_remove=false) ~msg repo =
  let args = ["commit"] in
  let args = if add_remove then
    args @ [ "-A"; ]
  else
    args
  in
  let args = args @ [ "-m"; msg; "--cwd"; Path.to_string repo] in
  let p = Process.exec "hg" args in
  ignore @@ assert_process_completes_ok ~msg:(Printf.sprintf "hg commit of %s" msg) p

let hg_update ~dest repo =
  let p = Process.exec "hg"
    ["update"; dest; "--cwd"; Path.to_string repo] in
  ignore @@ assert_process_completes_ok ~msg:(Printf.sprintf "hg update %s" dest) p

let init_hg_repo repo =
  let p = Process.exec "hg" ["init"; Path.to_string repo] in
  ignore @@ assert_process_completes_ok ~msg:"hg init" p;
  let hg_dir = Filename.concat (Path.to_string repo) ".hg" in
  let hgrc_file = Filename.concat hg_dir "hgrc" in
  let hgrc = Printf.sprintf "%s\n%s\n" "[extensions]" "fsmonitor =" in
  Sys_utils.write_file ~file:hgrc_file hgrc;
  let p = Process.exec "hg" ["add"; "--cwd"; Path.to_string repo] in
  ignore @@ assert_process_completes_ok ~msg:"hg add" p;
  hg_commit ~msg:"Starting" repo

let write_watchman_config ~content repo =
  let file = Filename.concat (Path.to_string repo) ".watchmanconfig" in
  Sys_utils.write_file ~file content

let write_file_to_repo ~file repo content =
  let file = Filename.concat (Path.to_string repo) file in
  Sys_utils.write_file ~file content

let rec poll_client_until_settled ?(retries=10) client =
  if retries < 0 then
    raise (Retries_exhausted "poll_client_until_settled")
  else
    if WEWClient.is_settled client then
      ()
    else
      let () = Sys_utils.sleep 1.0 in
      poll_client_until_settled ~retries:(retries - 1) client

let test_no_socket_file harness =
  let repo = harness.Test_harness.repo_dir in
  init_hg_repo repo;
  let watcher = WEWClient.init harness.Test_harness.repo_dir in
  if watcher = None then
    true
  else
    false

let test_no_watcher_running harness =
  let repo = harness.Test_harness.repo_dir in
  init_hg_repo repo;
  create_watcher_socket_file repo;
  let watcher = WEWClient.init harness.Test_harness.repo_dir in
  if watcher = None then
    true
  else
    false

let test_watcher_unknown_then_settled harness =
  let repo = harness.Test_harness.repo_dir in
  write_watchman_config ~content:"{}" repo;
  init_hg_repo repo;
  write_file_to_repo ~file:"abcde.php" repo "hello";
  hg_commit ~add_remove:true ~msg:"Add abcde" repo;
  WEW.spawn_daemon repo;
  let client = init_watcher_client ~retries:10 repo in
  let settled = WEWClient.is_settled client in
  BA.assert_equals false settled
    "Should not be settled when watcher first starts";
  let settled = WEWClient.is_settled client in
  BA.assert_equals false settled
    "Checking again, should still not be settled";
  hg_update ~dest:".~1" repo;
  poll_client_until_settled client;
  true

let tests args =
  let harness_config = {
    Test_harness.hh_server = args.Args.hh_server;
    hh_client = args.Args.hh_client;
    template_repo = args.Args.template_repo;
  } in
  [
  ("test_no_socket_file", fun () ->
    Test_harness.run_test ~stop_server_in_teardown:false harness_config
    test_no_socket_file);
  ("test_no_watcher_running", fun () ->
    Test_harness.run_test ~stop_server_in_teardown:false harness_config
    test_no_watcher_running);
  ("test_watcher_unknown_then_settled", fun () ->
    Test_harness.run_test ~stop_server_in_teardown:false harness_config
    test_watcher_unknown_then_settled);
  ]

let () =
  Daemon.check_entry_point ();
  let args = Args.parse () in
  let () = HackEventLogger.client_init (args.Args.template_repo) in
  Unit_test.run_all (tests args)
