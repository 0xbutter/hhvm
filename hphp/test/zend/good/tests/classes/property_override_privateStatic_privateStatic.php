<?php
class A
{
    private static $p = "A::p (static)";
    static function showA()
    {
        echo self::$p . "\n";
    }
}

class B extends A
{
    private static $p = "B::p (static)";
    static function showB()
    {
        echo self::$p . "\n";
    }
}

<<__EntryPoint>> function main() {
A::showA();

B::showA();
B::showB();
}
