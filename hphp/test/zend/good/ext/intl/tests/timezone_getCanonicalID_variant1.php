<?php <<__EntryPoint>> function main() {
ini_set("intl.error_level", E_WARNING);

var_dump(IntlTimeZone::getCanonicalID('Portugal', $isSystemId));
var_dump($isSystemId);

var_dump(IntlTimeZone::getCanonicalID('GMT +01:25', $isSystemId));
var_dump($isSystemId);
echo "==DONE==";
}
