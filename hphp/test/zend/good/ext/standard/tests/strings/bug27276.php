<?php <<__EntryPoint>> function main() {
ini_set("memory_limit", "18m");
$replacement = str_repeat("x", 12444);
$string = str_repeat("x", 9432);
$key =    "{BLURPS}";

str_replace($key, $replacement, $string);

echo "Alive!\n";
}
