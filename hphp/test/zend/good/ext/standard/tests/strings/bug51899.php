<?php
<<__EntryPoint>> function main() {
var_dump(parse_ini_string('a='));
var_dump(parse_ini_string('a= '));
var_dump(parse_ini_string('a='.PHP_EOL));
var_dump(parse_ini_string('a=b '));
var_dump(parse_ini_string(''));
var_dump(parse_ini_string("\0"));
}
