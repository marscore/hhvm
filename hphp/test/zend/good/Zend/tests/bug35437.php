<?php

function err2exception($errno, $errstr)
{
    throw new Exception("Error occuried: " . $errstr);
}

class TestClass
{
    function testMethod()
    {
        $GLOBALS['t'] = new stdClass;
    }
}
<<__EntryPoint>> function main() {
set_error_handler('err2exception');

try {
    TestClass::testMethod();
} catch (Exception $e) {
    echo "Catched: ".$e->getMessage()."\n";
}
}
