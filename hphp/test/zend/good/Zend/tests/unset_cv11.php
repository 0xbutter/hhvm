<?php <<__EntryPoint>> function main() {
$x = array("default"=>"ok");
var_dump($x);
$cf = $x;
unset($cf['default']);
var_dump($x);
echo "ok\n";
}
