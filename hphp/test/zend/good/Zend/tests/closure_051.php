<?php

class A {
    static function foo() {
        $f = static function() {
            return static::class;
        };
        return $f();
    }
}

class B extends A {}
<<__EntryPoint>> function main() {
var_dump(B::foo());
}
