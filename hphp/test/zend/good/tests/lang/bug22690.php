<?php
    $foo = $s ==> strtoupper($s);
    ob_start($foo);
    echo $foo("bar\n");
<<__EntryPoint>> function main() {
echo "bar";
}
