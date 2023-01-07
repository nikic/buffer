--TEST--
Buffer BasicTests - Empty file
--DESCRIPTION--
The extension being loaded should work
--CREDITS--
Pablo Duboue pablo.duboue@gmail.com
--SKIPIF--
<?php if(!extension_loaded('buffer')) die('skip buffer n/a'); ?>
--FILE--
<?php
echo "loaded\n";
?>
--EXPECT--
loaded
