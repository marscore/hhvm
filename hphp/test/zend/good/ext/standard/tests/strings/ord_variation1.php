<?php

/* Prototype  : int ord  ( string $string  )
 * Description: Return ASCII value of character
 * Source code: ext/standard/string.c
*/

//defining a class
class sample  {
  public function __toString() {
    return "sample object";
  }
}
<<__EntryPoint>> function main() {
echo "*** Testing ord() function: with unexpected inputs for 'string' argument ***\n";

//get an unset variable
$unset_var = 'string_val';
unset($unset_var);

//getting the resource
$file_handle = fopen(__FILE__, "r");

// array with different values for $input
$inputs =  array (

/*1*/      0,
          1,
          255,
          256,
          2147483647,
          -2147483648,

          // float values
/*7*/      10.5,
          -20.5,
          10.1234567e10,

          // array values
/*10*/      array(),
          array(0),
          array(1, 2),

          // boolean values
/*13*/      true,
          false,
          TRUE,
          FALSE,

          // null values
/*17*/      NULL,
          null,

          // objects
/*19*/      new sample(),

          // resource
/*20*/      $file_handle,

          // undefined variable
/*21*/      @$undefined_var,

          // unset variable
/*22*/      @$unset_var
);

// loop through with each element of the $string array to test ord() function
$count = 1;
foreach($inputs as $input) {
  echo "-- Iteration $count --\n";
  try { var_dump( ord($input) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count ++;
}

fclose($file_handle);  //closing the file handle

echo "===DONE===\n";
}
