<?hh <<__EntryPoint>> function main() {
$data = file(__FILE__);
try {
  foreach ($data as $line) {
    throw new Exception("error");
  }
} catch (Exception $e) {
  echo "ok\n";
}
}
