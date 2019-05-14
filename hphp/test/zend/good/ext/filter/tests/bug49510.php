<?php <<__EntryPoint>> function main() {
var_dump(filter_var(false, FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE));
var_dump(filter_var(0, FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE));
var_dump(filter_var("0", FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE));
var_dump(filter_var("off", FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE));
var_dump(filter_var("", FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE));
var_dump(filter_var("false", FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE));
var_dump(filter_var("no", FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE));

var_dump(filter_var(true, FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE));
var_dump(filter_var(1, FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE));
var_dump(filter_var("1", FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE));
var_dump(filter_var("on", FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE));
var_dump(filter_var("yes", FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE));

var_dump(filter_var("invalid", FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE));
echo "==DONE==";
}
