<?php
function test ($b) {
    $b++;
    return($b);
}
<<__EntryPoint>> function main() {
$a = test(1);
echo $a;
}
