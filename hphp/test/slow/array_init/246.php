<?php

function f() {
 throw new Exception();
 }
function test() {
  $a = array(1, f(), 2, f(), 3);
  var_dump($a);
}

<<__EntryPoint>>
function main_246() {
try {
 test();
 }
 catch (Exception $e) {
 }
}
