<?php
<<__EntryPoint>> function main() {
try {

	$dll = new SplQueue();
	$dll->setIteratorMode(SplDoublyLinkedList::IT_MODE_LIFO);

} catch (Exception $e) {
	echo $e->getMessage();
}
}
