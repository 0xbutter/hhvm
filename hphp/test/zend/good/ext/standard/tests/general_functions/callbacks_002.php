<?php
<<__EntryPoint>> function main() {
call_user_func(array('Foo', 'bar'));
call_user_func(array(NULL, 'bar'));
call_user_func(array('stdclass', NULL));
}
