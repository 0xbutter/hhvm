<?php
<<__EntryPoint>> function main() {
echo gettype (Imagick::queryFormats ()) . PHP_EOL;
echo gettype (Imagick::queryFonts ()) . PHP_EOL;
echo 'success';
}
