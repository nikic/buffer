--TEST--
Buffer BasicTests - Two views Test
--DESCRIPTION--
Test whether two overlapping views can access the same buffer
--CREDITS--
Pablo Duboue pablo.duboue@gmail.com
--SKIPIF--
<?php if(!extension_loaded('buffer')) die('skip buffer n/a'); ?>
--FILE--
<?php

$buffer = new ArrayBuffer(10 * 8 * 20);
$v1 = new DoubleArray($buffer, 0, 4);
$v2 = new DoubleArray($buffer, 2 * 8, 6);
for($i=0; $i<4; $i++){
  $v1[$i] = $i / 2.0;
}
for($i=0; $i<6; $i++){
  $v2[$i] = $i / 3.0;
}

print_r($v1);
print_r($v2);
?>
--EXPECT--
DoubleArray Object
(
    [buffer] => ArrayBuffer Object
        (
        )

    [offset] => 0
    [length] => 4
    [0] => 0
    [1] => 0.5
    [2] => 0
    [3] => 0.33333333333333
)
DoubleArray Object
(
    [buffer] => ArrayBuffer Object
        (
        )

    [offset] => 16
    [length] => 6
    [0] => 0
    [1] => 0.33333333333333
    [2] => 0.66666666666667
    [3] => 1
    [4] => 1.3333333333333
    [5] => 1.6666666666667
)
