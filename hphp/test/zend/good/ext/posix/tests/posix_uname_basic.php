<?hh
  echo "Basic test of POSIX uname function\n";

  $uname = posix_uname();
  unset($uname['domainname']);
  print_r($uname);
<<__EntryPoint>> function main() {
echo "===DONE====";
}
