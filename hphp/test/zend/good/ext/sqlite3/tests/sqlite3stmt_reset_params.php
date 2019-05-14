<?php <<__EntryPoint>> function main() {
$db = new SQLite3(':memory:');
echo "Creating Table\n";
var_dump($db->exec('CREATE TABLE foobar (id INTEGER, name STRING)'));

echo "INSERT into table\n";
var_dump($db->exec("INSERT INTO foobar (id, name) VALUES (1, 'john')"));


$query = "SELECT name FROM foobar WHERE id = 1";

echo "Prepare query\n";
$stmt = $db->prepare($query);

echo "Reset query\n";
try { $stmt->reset("foo");  } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Closing database\n";
$stmt = null;
$result = null;
var_dump($db->close());
echo "Done\n";
}
