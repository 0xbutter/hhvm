<?php
<<__EntryPoint>> function main() {
$a = $x ==> function($y) use ($x) { return $x * $y; };

var_dump($a(2)->__invoke(4));
}
