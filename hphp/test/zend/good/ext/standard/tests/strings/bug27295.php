<?php <<__EntryPoint>> function main() {
$strings = array("foo = bar", "bar = foo");
foreach( $strings as $string )
{
    list($var, $val) = sscanf( $string, "%s = %[^[]]");
    echo "$var = $val\n";
}
}
