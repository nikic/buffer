--TEST--
Buffer BasicTests - Simple Test
--DESCRIPTION--
Test access
--SKIPIF--
<?php if(!extension_loaded('buffer')) die('skip buffer n/a'); ?>
--FILE--
<?php

/* First create an ArrayBuffer with a certain size (in this example 8MB). The array buffer itself
 * is just a chunk of binary data with no associated meaning. */
echo "1\n";
$buffer = new ArrayBuffer(1024 * 1024 * 8);

/* Create an int32 view on the buffer. This will interpret every four bytes of the buffer as one
 * signed 32-bit integer. It can be used as a normal array (obviously only in the allowed range) */
echo "2\n";
$int32s = new Int32Array($buffer);

echo "3\n";
$int32s[0] = 1234;
echo "4\n";
var_dump($int32s[0]); // int(1234)

/* We can interpret the *same* buffer in a different way, namely as an array of floats */
echo "5\n";
$floats = new FloatArray($buffer);

/* Here for example we are reinterpreting the 1234 int32 integer as a single-precision floating
 * point number. */
echo "6\n";
var_dump($floats[0]); // float(1.7292023049768E-42)

/* It is also possible to restrict the view to a certain part of the buffer. E.g. here we create
 * an unsigned 16-bit integer view that starts 32 bytes into the buffer and contains 1024 elements.
 */
echo "7\n";
$uint16s = new UInt16Array($buffer, 32, 1024);
echo "8\n";
?>
--EXPECTF--
1
2
3
4
int(1234)
5
6
float(1.7292023049768%SE-42)
7
8

