<?php

trait MY_TRAIT {
 }
class MY_CLASS {
 use MY_TRAIT;
 }
<<__EntryPoint>> function main() {
$r = new ReflectionClass('MY_CLASS');
var_dump($r->getTraitNames());
}
