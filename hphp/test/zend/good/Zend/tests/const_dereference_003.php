<?php <<__EntryPoint>> function main() {
error_reporting(E_ALL);

var_dump([1, 2, 3, 4,][3]);
var_dump([1, 2, 3, 4]['foo']);
var_dump([array(1,2,3), [4, 5, 6]][1][2]);

foreach (array([1, 2, 3])[0] as $var) {
     echo $var;
}
}
