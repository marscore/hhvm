<?php
class TEST { const FOO = SEEK_CUR; };
class TEST2 { const FOO = 1; };
class TEST3 { const FOO = PHP_VERSION; };
<<__EntryPoint>> function main() {
print TEST::FOO."\n";
}
