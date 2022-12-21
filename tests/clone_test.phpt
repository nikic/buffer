--TEST--
Buffer BasicTests - Test clone
--DESCRIPTION--
Take a buffer and a view and clone them
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
print_r($v1);
$buffer2 = clone($buffer);
$v2 = new DoubleArray($buffer2, 0, 20);
print_r($v2);
$v1[0] = 42.0;
echo $v2[0]."\n";
$v3 = clone($v2);
$v3[0] = 42.0;
echo $v2[0]."\n";
?>
--EXPECT--
DoubleArray Object
(
    [buffer] => ArrayBuffer Object
        (
        )

    [offset] => 0
    [length] => 20
    [0] => 0
    [1] => 0.5
    [2] => 1
    [3] => 1.5
    [4] => 2
    [5] => 2.5
    [6] => 3
    [7] => 3.5
    [8] => 4
    [9] => 4.5
    [10] => 5
    [11] => 5.5
    [12] => 6
    [13] => 6.5
    [14] => 7
    [15] => 7.5
    [16] => 8
    [17] => 8.5
    [18] => 9
    [19] => 9.5
)
DoubleArray Object
(
    [buffer] => ArrayBuffer Object
        (
        )

    [offset] => 0
    [length] => 20
    [0] => 0
    [1] => 0.5
    [2] => 1
    [3] => 1.5
    [4] => 2
    [5] => 2.5
    [6] => 3
    [7] => 3.5
    [8] => 4
    [9] => 4.5
    [10] => 5
    [11] => 5.5
    [12] => 6
    [13] => 6.5
    [14] => 7
    [15] => 7.5
    [16] => 8
    [17] => 8.5
    [18] => 9
    [19] => 9.5
)
0
42
