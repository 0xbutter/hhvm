<?php
<<__EntryPoint>> function main() {
$im = new Imagick;
$im->readImage(__DIR__ . '/facebook.png');
var_dump($im->getImageMimeType());
}
