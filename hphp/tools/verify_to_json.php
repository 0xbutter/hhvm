#!/bin/env php
<?php

$HPHP_HOME = $_ENV['HPHP_HOME'];
include_once $HPHP_HOME.'/hphp/test/fbmake_test_lib.php';

//////////////////////////////////////////////////////////////////////

if (count($argv) != 6) {
  echo "usage: $argv[0] test-script TEST_PATH interp|jit|hhir REPO_BOOLEAN FBMAKE_BIN_ROOT
    FBMAKE_BIN_ROOT\n";
  exit(1);
}

$cmd = "TEST_PATH=$argv[2] VQ=$argv[3] REPO=$argv[4] FBMAKE_BIN_ROOT=$HPHP_HOME/$argv[5] " .
       "$HPHP_HOME/hphp/tools/$argv[1]";

loop_tests($cmd, function ($line) {
  if (preg_match('/^(test[^\s]*).*/', $line, &$m)) {
    start($m[1]);
    return;
  }
  if (!test_is_running()) return;
  if (preg_match('/^\s*passed.*/', $line)) {
    finish('passed');
  } else if (preg_match('/^[\s\*]*FAILED.*/', $line)) {
    finish('failed');
  }
});
