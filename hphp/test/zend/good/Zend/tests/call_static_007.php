<?php

class a {
    public function __call($a, $b) {
        print "__call: ". $a ."\n";
    }
    public function baz() {
        self::Bar();
    }
}

<<__EntryPoint>> function main() {
$a = new a;

$b = 'Test';
$a->$b();

$a->baz();
}
