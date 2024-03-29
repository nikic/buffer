--TEST--
Various error conditions
--SKIPIF--
<?php if(!extension_loaded('buffer')) die('skip buffer n/a'); ?>
--FILE--
<?php

try {
    new ArrayBuffer(-1);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}

$buffer = new ArrayBuffer(8);
try {
    new Int8Array($buffer, -1);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}
try {
    new Int8Array($buffer, 0, -1);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}
try {
    new Int8Array($buffer, 9, 1);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}
try {
    new Int8Array($buffer, 7, 2);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}

?>
--EXPECT--
ArrayBuffer::__construct(): Argument #1 ($byteLength) must be greater than or equal to 0
TypedArray::__construct(): Argument #2 ($byteOffset) must be greater than or equal to 0
TypedArray::__construct(): Argument #3 ($length) must be greater than or equal to 0
TypedArray::__construct(): Argument #2 ($byteOffset) must be smaller than or equal to the buffer length
TypedArray::__construct(): Argument #3 ($length) must be smaller than or equal to the buffer length
