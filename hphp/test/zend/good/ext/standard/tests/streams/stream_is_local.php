<?php
<<__EntryPoint>> function main() {
$a = 1;
$b = $a;
var_dump(stream_is_local($b));
var_dump($b);

var_dump(stream_is_local(fopen(__FILE__, 'r')));
}
