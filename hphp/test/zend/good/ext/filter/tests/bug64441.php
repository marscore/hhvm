<?php <<__EntryPoint>> function main() {
var_dump(filter_var('http://example.com./', FILTER_VALIDATE_URL));
var_dump(filter_var('http://example.com/', FILTER_VALIDATE_URL));
}
