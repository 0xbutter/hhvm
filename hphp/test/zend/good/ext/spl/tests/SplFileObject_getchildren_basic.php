<?hh <<__EntryPoint>> function main() {
$s = new SplFileObject( __FILE__ );
var_dump($s->getChildren());
}
