<?hh <<__EntryPoint>> function main() {
$c = array('a' => 'aa','b' => 'bb');

var_dump(array_merge_recursive($c, $c));
}
