--TEST--
Buffer BasicTests - Test equality
--DESCRIPTION--
Take a buffer and a view clones and compare them
--CREDITS--
Pablo Duboue pablo.duboue@gmail.com
--SKIPIF--
<?php if(!extension_loaded('buffer')) die('skip buffer n/a'); ?>
--FILE--
<?php
$buffer = new ArrayBuffer(8 * 20);
$v1 = new DoubleArray($buffer, 0, 20);
for($i=0; $i<20; $i++){
    $v1[$i] = $i / 2.0;
}
$buffer2 = clone($buffer);
$v2 = new DoubleArray($buffer2, 0, 20);
if($v1 == $v2) {
  echo "same\n";
}else{
  echo "different\n";
}
$v1[0] = 42.0;
if($v1 == $v2) {
  echo "same\n";
}else{
  echo "different\n";
}
$v3 = clone($v2);
$v3[0] = 42.0;
if($v1 == $v2) {
  echo "same\n";
}else{
  echo "different\n";
}
?>
--EXPECT--
same
different
same
