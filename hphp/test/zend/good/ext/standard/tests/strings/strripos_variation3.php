<?php
/* Prototype  : int strripos ( string $haystack, string $needle [, int $offset] );
 * Description: Find position of last occurrence of a case-insensitive 'needle' in a 'haystack'
 * Source code: ext/standard/string.c
*/

/* Test strripos() function by passing multi-line heredoc string for haystack and 
 *  with various needles & offsets
*/
<<__EntryPoint>> function main() {
echo "*** Testing strripos() function: with heredoc strings ***\n";
echo "-- With heredoc string containing multi lines --\n";
$multi_line_str = <<<EOD
Example of string
spanning multiple lines
using heredoc syntax.
EOD;

echo "\n-- Multi line strings with +ve offsets -- \n";
var_dump( strripos($multi_line_str, "iNg", 0) );
var_dump( strripos($multi_line_str, "inG", 15) );
var_dump( strripos($multi_line_str, "Ing", 22) );

echo "\n-- Multi line strings with +ve offsets -- \n";
var_dump( strripos($multi_line_str, "Ing", -1) );
var_dump( strripos($multi_line_str, "Ing", -17) );
var_dump( strripos($multi_line_str, "Ing", -50) );

echo "\n-- Multi line strings with no offset -- \n";
var_dump( strripos($multi_line_str, "spAn") );
var_dump( strripos($multi_line_str, "IPlE") );
var_dump( strripos($multi_line_str, "") );
var_dump( strripos($multi_line_str, " ") );

echo "===DONE===\n";
}
