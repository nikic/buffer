Typed array implementation using buffers
----------------------------------------

This PHP extension provides typed arrays based on JavaScript's ArrayBuffer interface.

This extension has been written primarily as a small example for a tutorial and not for its actual
functionality. That's also why it does not contain the full functionality of JS ArrayBuffers. I may
add the remaining functionality lateron.

Usage
-----

```php
<?php

/* First create an ArrayBuffer with a certain size (in this example 8MB). The array buffer itself
 * is just a chunk of binary data with no associated meaning. */
$buffer = new ArrayBuffer(1024 * 1024 * 8);

/* Create an int32 view on the buffer. This will interpret every four bytes of the buffer as one
 * signed 32-bit integer. It can be used as a normal array (obviously only in the allowed range) */
$int32s = new Int32Array($buffer);

$int32s[0] = 1234;
var_dump($int32s[0]); // int(1234)

/* We can interpret the *same* buffer in a different way, namely as an array of floats */
$floats = new FloatArray($buffer);

/* Here for example we are reinterpreting the 1234 int32 integer as a single-precision floating
 * point number. */
var_dump($floats[0]); // float(1.7292023049768E-42)

/* It is also possible to restrict the view to a certain part of the buffer. E.g. here we create
 * an unsigned 16-bit integer view that starts 32 bytes into the buffer and contains 1024 elements.
 */
$uint16s = new UInt16Array($buffer, 32, 1024);
```

The following buffer views are available:

    Int8Array
    UInt8Array
    Int16Array
    Uint16Array
    Int32Array
    UInt32Array
    FloatArray
    DoubleArray

Installation
------------

The extension is installed as usual:

    ./configure --enable-buffer
    make
    make install
