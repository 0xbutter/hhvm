<?php

function foo() {
    echo $undef;
    error_reporting(E_ALL|E_STRICT);
}
<<__EntryPoint>> function main() {
error_reporting(E_ALL);

foo(@$var);

var_dump(error_reporting());

echo "Done\n";
}
