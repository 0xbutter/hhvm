<?php

namespace foo;

interface IFoo { }

interface ITest extends IFoo { }

interface IBar extends IFoo { }

<<__EntryPoint>> function main() {
\var_dump(\interface_exists('IFoo'));
\var_dump(\interface_exists('foo\\IFoo'));
\var_dump(\interface_exists('FOO\\ITEST'));
}
