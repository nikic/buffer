--TEST--
Buffer BasicTests - Test view serialization
--DESCRIPTION--
Take a view and serialize it
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
$serstr = serialize($v1);
echo strlen($serstr)."\n";

$v2 = unserialize($serstr);
foreach($v2 as $v){
  echo "$v\n";
}
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
271
0
0.5
1
1.5
2
2.5
3
3.5
4
4.5
5
5.5
6
6.5
7
7.5
8
8.5
9
9.5
