--TEST--
Zero-length ArrayBuffer
--SKIPIF--
<?php if(!extension_loaded('buffer')) die('skip buffer n/a'); ?>
--FILE--
<?php

$buf = new ArrayBuffer(0);
var_dump($buf);
var_dump(clone $buf);
var_dump($s = serialize($buf));
var_dump(unserialize($s));

$i8 = new Int8Array($buf);
try {
    $i8[0];
} catch (Exception $e) {
    echo $e->getMessage(), "\n";
}

$buf = new ArrayBuffer(8);
$i8 = new Int8Array($buf, 8);
try {
    $i8[0];
} catch (Exception $e) {
    echo $e->getMessage(), "\n";
}

$i8 = new Int8Array($buf, 4, 0);
try {
    $i8[0];
} catch (Exception $e) {
    echo $e->getMessage(), "\n";
}

?>
--EXPECT--
object(ArrayBuffer)#1 (0) {
}
object(ArrayBuffer)#2 (0) {
}
string(41) "O:11:"ArrayBuffer":1:{s:4:"data";s:0:"";}"
object(ArrayBuffer)#2 (0) {
}
Offset is outside the buffer range
Offset is outside the buffer range
Offset is outside the buffer range
