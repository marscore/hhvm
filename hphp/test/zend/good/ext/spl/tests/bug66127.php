<?php
function crash()
{
    set_error_handler(function () {});
    $var = 1;
    trigger_error('error');
    $var2 = $var;
    $var3 = $var;
    trigger_error('error');
}
<<__EntryPoint>> function main() {
$items = new ArrayObject();

unset($items[0]);
unset($items[0][0]);
crash();
echo "Worked!\n";
}
