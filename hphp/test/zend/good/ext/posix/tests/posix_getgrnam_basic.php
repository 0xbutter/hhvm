<?php <<__EntryPoint>> function main() {
echo "Basic test of POSIX posix_getgrnam function\n";

var_dump(posix_getgrnam('1'));
var_dump(posix_getgrnam(''));

echo "===DONE===\n";
}
