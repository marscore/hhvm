<?php

trait foo {
    public function a() {
    }
}

interface baz {
    public function abc();
}

class bar implements baz {
    use foo;

}
<<__EntryPoint>> function main() {
new bar;
}
