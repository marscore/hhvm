<?php
/* Prototype  : proto bool xml_set_processing_instruction_handler  ( resource $parser  , callback $handler  )
 * Description: Sets the processing instruction (PI) handler function for the XML parser.
 * Source code: ext/xml/xml.c
 * Alias to functions:
 */

class XML_Parser
{

    function PIHandler($parser, $target, $data)
    {
        echo "Target: " . $target. "\n";
        echo "Data: " . $data . "\n";
    }

    function parse($data)
    {
        $parser = xml_parser_create();
        $thiz = $this;
        xml_set_object($parser, &$thiz);
        xml_set_processing_instruction_handler($parser, "PIHandler");
        xml_parse($parser, $data, true);
        xml_parser_free($parser);
    }


}
<<__EntryPoint>> function main() {
$xml = <<<HERE
<?xml version="1.0" encoding="ISO-8859-1"?>
<?xml-stylesheet href="default.xsl" type="text/xml"?>
HERE;

echo "Simple test of xml_set_processing_instruction_handler() function\n";
$p1 = new Xml_Parser();
$p1->parse($xml);
echo "Done\n";
}
