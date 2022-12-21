--TEST--
Buffer BasicTests - Simple buffer creation
--DESCRIPTION--
The created buffer should work.
--CREDITS--
Pablo Duboue pablo.duboue@gmail.com
--SKIPIF--
<?php if(!extension_loaded('buffer')) die('skip buffer n/a'); ?>
--FILE--
<?php

echo "loaded\n";

$buffer = new ArrayBuffer(1024 * 1024 * 8);

echo "allocated\n";
--EXPECT--
loaded
allocated
