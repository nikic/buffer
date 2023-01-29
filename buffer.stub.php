<?php
/**
 * @generate-class-entries
 */

/** @strict-properties */
final class ArrayBuffer {
    public function __construct(int $length) {}

    public function __serialize(): array {}

    public function __unserialize(array $data): void {}
}

/** @strict-properties */
abstract class TypedArray implements ArrayAccess, Iterator {
    /** @implementation-alias TypedArray::__construct */
    public function __construct(ArrayBuffer $buffer, int $offset = 0, int $length = 0) {}

    /**
     * @param int $offset
     */
    public function offsetGet($offset): int|float {}

    /**
     * @param int $offset
     * @param int|float $value
     */
    public function offsetSet($offset, $value): void {}

    /**
     * @param int $offset
     */
    public function offsetExists($offset): bool {}

    /**
     * @param int $offset
     */
    public function offsetUnset($offset): void {}

    public function rewind(): void {}

    public function next(): void {}

    public function valid(): bool {}

    public function key(): int {}

    public function current(): int|float {}

    public function __serialize(): array {}

    public function __unserialize(array $data): void {}
}

final class Int8Array extends TypedArray {
    /**
     * @param int $offset
     * @implementation-alias TypedArray::offsetGet
     */
    public function offsetGet($offset): int {}

    /** @implementation-alias TypedArray::current */
    public function current(): int {}
}

final class UInt8Array extends TypedArray {
    /**
     * @param int $offset
     * @implementation-alias TypedArray::offsetGet
     */
    public function offsetGet($offset): int {}

    /** @implementation-alias TypedArray::current */
    public function current(): int {}
}

final class Int16Array extends TypedArray {
    /**
     * @param int $offset
     * @implementation-alias TypedArray::offsetGet
     */
    public function offsetGet($offset): int {}

    /** @implementation-alias TypedArray::current */
    public function current(): int {}
}

final class UInt16Array extends TypedArray {
    /**
     * @param int $offset
     * @implementation-alias TypedArray::offsetGet
     */
    public function offsetGet($offset): int {}

    /** @implementation-alias TypedArray::current */
    public function current(): int {}
}

final class Int32Array extends TypedArray {
    /**
     * @param int $offset
     * @implementation-alias TypedArray::offsetGet
     */
    public function offsetGet($offset): int {}

    /** @implementation-alias TypedArray::current */
    public function current(): int {}
}

final class UInt32Array extends TypedArray {
}

final class FloatArray extends TypedArray {
    /**
     * @param int $offset
     * @implementation-alias TypedArray::offsetGet
     */
    public function offsetGet($offset): float {}

    /** @implementation-alias TypedArray::current */
    public function current(): float {}
}

final class DoubleArray extends TypedArray {
    /**
     * @param int $offset
     * @implementation-alias TypedArray::offsetGet
     */
    public function offsetGet($offset): float {}

    /** @implementation-alias TypedArray::current */
    public function current(): float {}
}
