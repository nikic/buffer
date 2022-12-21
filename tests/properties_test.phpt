--TEST--
Buffer BasicTests - Test basic properties
--DESCRIPTION--
Take whether length works
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
var_dump($buffer);
var_dump($v1);
?>
--EXPECT--
object(ArrayBuffer)#1 (0) {
}
object(DoubleArray)#2 (23) {
  ["buffer"]=>
  object(ArrayBuffer)#1 (0) {
  }
  ["offset"]=>
  int(0)
  ["length"]=>
  int(20)
  [0]=>
  float(0)
  [1]=>
  float(0.5)
  [2]=>
  float(1)
  [3]=>
  float(1.5)
  [4]=>
  float(2)
  [5]=>
  float(2.5)
  [6]=>
  float(3)
  [7]=>
  float(3.5)
  [8]=>
  float(4)
  [9]=>
  float(4.5)
  [10]=>
  float(5)
  [11]=>
  float(5.5)
  [12]=>
  float(6)
  [13]=>
  float(6.5)
  [14]=>
  float(7)
  [15]=>
  float(7.5)
  [16]=>
  float(8)
  [17]=>
  float(8.5)
  [18]=>
  float(9)
  [19]=>
  float(9.5)
}