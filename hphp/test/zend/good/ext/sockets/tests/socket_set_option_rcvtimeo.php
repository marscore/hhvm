<?php <<__EntryPoint>> function main() {
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if (!$socket) {
        die('Unable to create AF_INET socket [socket]');
}
socket_set_block($socket);

//wrong params
$retval_1 = socket_set_option( $socket, SOL_SOCKET, SO_RCVTIMEO, array());

//set/get comparison
$options = array("sec" => 1, "usec" => 0);
$retval_2 = socket_set_option( $socket, SOL_SOCKET, SO_RCVTIMEO, $options);
$retval_3 = socket_get_option( $socket, SOL_SOCKET, SO_RCVTIMEO);

var_dump($retval_2);
var_dump($retval_3 === $options);
socket_close($socket);
}
