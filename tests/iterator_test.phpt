--TEST--
Buffer BasicTests - Iterator Test
--DESCRIPTION--
Test access
--CREDITS--
Pablo Duboue pablo.duboue@gmail.com
--SKIPIF--
<?php if(!extension_loaded('buffer')) die('skip buffer n/a'); ?>
--FILE--
<?php

$buffer = new ArrayBuffer(10 * 4);
$int32s = new Int32Array($buffer);
for($i=0; $i<10; $i++){
  echo "$i\n";
  $int32s[$i] = $i;
}

foreach ($int32s as $k => $v) {
  echo "$k, $v\n";
}

$it = $int32s->getIterator();
while ($it->valid()) $it->next();
var_dump($it->key());
var_dump($it->current());

?>
--EXPECT--
0
1
2
3
4
5
6
7
8
9
0, 0
1, 1
2, 2
3, 3
4, 4
5, 5
6, 6
7, 7
8, 8
9, 9
int(10)
NULL
