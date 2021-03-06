<?php <<__EntryPoint>> function main() {
var_dump(libxml_use_internal_errors(true));

$xmlstr = <<< XML
<?xml version='1.0' standalone='yes'?>
    <movies>
        <movie>
            <titles>PHP: Behind the Parser</title>
        </movie>
    </movies>
XML;

simplexml_load_string($xmlstr);

// test memleaks here
var_dump(libxml_use_internal_errors(false));

echo "Done\n";
}
