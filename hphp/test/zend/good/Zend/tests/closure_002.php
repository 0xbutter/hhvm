<?php
<<__EntryPoint>> function main() {
$x = 4;

$lambda1 = function () use ($x) {
	echo "$x\n";
};

$lambda1();
$x++;
$lambda1();

echo "Done\n";
}
