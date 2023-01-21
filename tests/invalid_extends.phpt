--TEST--
Check that custom TypedArray extension doesn't work
--SKIPIF--
<?php if(!extension_loaded('buffer')) die('skip buffer n/a'); ?>
--FILE--
<?php
class Foo extends TypedArray {}
new Foo;
?>
--EXPECTF--
Fatal error: Trying to instantiate an invalid TypedArray extension in %s on line %d
