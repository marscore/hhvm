<?php <<__EntryPoint>> function main() {
$doc = new DOMDocument();
$xpath = new DOMXPath($doc);
$nodes = $xpath->query('*');
echo get_class($nodes), "\n";
var_dump($nodes->length);
$length = $nodes->length;
var_dump(empty($nodes->length), empty($length));

$doc->loadXML("<element></element>");
var_dump($doc->firstChild->nodeValue, empty($doc->firstChild->nodeValue), isset($doc->firstChild->nodeValue));
var_dump(empty($doc->nodeType), empty($doc->firstChild->nodeType));
echo "===DONE===\n";
}
