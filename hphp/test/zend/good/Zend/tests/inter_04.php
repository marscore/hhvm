<?php

interface a {
    function b();
}

interface b {
    function b();
}

interface c extends a, b {
}
<<__EntryPoint>> function main() {
echo "done!\n";
}
