<?php <<__EntryPoint>> function main() {
ini_set("intl.error_level", E_WARNING);
$ams = IntlTimeZone::createTimeZone('Europe/Amsterdam');
var_dump($ams->getRawOffset());

$lsb = IntlTimeZone::createTimeZone('Europe/Lisbon');
var_dump(intltz_get_raw_offset($lsb));
echo "==DONE==";
}
