<?php
/**
 * @generate-function-entries
 * @generate-legacy-arginfo
 */

class ArrayBuffer implements Serializable {
    public function __construct(int $length) {}

    public function serialize(): string {}

    public function unserialize(string $data): void {}

    public function __serialize(): array {}

    public function __unserialize(array $data): void {}
}

class Int8Array implements ArrayAccess, Iterator {
    /** @implementation-alias array_buffer_view_ctor */
    public function __construct(ArrayBuffer $buffer) {}

    /** @implementation-alias array_buffer_view_wakeup */
    public function __wakeup(): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_get
     */
    public function offsetGet($offset): int {}

    /**
     * @param int $offset
     * @param int $value
     * @implementation-alias array_buffer_view_offset_set
     */
    public function offsetSet($offset, $value): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_exists
     */
    public function offsetExists($offset): bool {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_unset
     */
    public function offsetUnset($offset): void {}

    /** @implementation-alias array_buffer_view_rewind */
    public function rewind(): void {}

    /** @implementation-alias array_buffer_view_next */
    public function next(): void {}

    /** @implementation-alias array_buffer_view_valid */
    public function valid(): bool {}

    /** @implementation-alias array_buffer_view_key */
    public function key(): int {}

    /** @implementation-alias array_buffer_view_current */
    public function current(): int {}
}

class UInt8Array implements ArrayAccess, Iterator {
    /** @implementation-alias array_buffer_view_ctor */
    public function __construct(ArrayBuffer $buffer) {}

    /** @implementation-alias array_buffer_view_wakeup */
    public function __wakeup(): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_get
     */
    public function offsetGet($offset): int {}

    /**
     * @param int $offset
     * @param int $value
     * @implementation-alias array_buffer_view_offset_set
     */
    public function offsetSet($offset, $value): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_exists
     */
    public function offsetExists($offset): bool {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_unset
     */
    public function offsetUnset($offset): void {}

    /** @implementation-alias array_buffer_view_rewind */
    public function rewind(): void {}

    /** @implementation-alias array_buffer_view_next */
    public function next(): void {}

    /** @implementation-alias array_buffer_view_valid */
    public function valid(): bool {}

    /** @implementation-alias array_buffer_view_key */
    public function key(): int {}

    /** @implementation-alias array_buffer_view_current */
    public function current(): int {}
}

class Int16Array implements ArrayAccess, Iterator {
    /** @implementation-alias array_buffer_view_ctor */
    public function __construct(ArrayBuffer $buffer) {}

    /** @implementation-alias array_buffer_view_wakeup */
    public function __wakeup(): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_get
     */
    public function offsetGet($offset): int {}

    /**
     * @param int $offset
     * @param int $value
     * @implementation-alias array_buffer_view_offset_set
     */
    public function offsetSet($offset, $value): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_exists
     */
    public function offsetExists($offset): bool {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_unset
     */
    public function offsetUnset($offset): void {}

    /** @implementation-alias array_buffer_view_rewind */
    public function rewind(): void {}

    /** @implementation-alias array_buffer_view_next */
    public function next(): void {}

    /** @implementation-alias array_buffer_view_valid */
    public function valid(): bool {}

    /** @implementation-alias array_buffer_view_key */
    public function key(): int {}

    /** @implementation-alias array_buffer_view_current */
    public function current(): int {}
}

class UInt16Array implements ArrayAccess, Iterator {
    /** @implementation-alias array_buffer_view_ctor */
    public function __construct(ArrayBuffer $buffer) {}

    /** @implementation-alias array_buffer_view_wakeup */
    public function __wakeup(): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_get
     */
    public function offsetGet($offset): int {}

    /**
     * @param int $offset
     * @param int $value
     * @implementation-alias array_buffer_view_offset_set
     */
    public function offsetSet($offset, $value): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_exists
     */
    public function offsetExists($offset): bool {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_unset
     */
    public function offsetUnset($offset): void {}

    /** @implementation-alias array_buffer_view_rewind */
    public function rewind(): void {}

    /** @implementation-alias array_buffer_view_next */
    public function next(): void {}

    /** @implementation-alias array_buffer_view_valid */
    public function valid(): bool {}

    /** @implementation-alias array_buffer_view_key */
    public function key(): int {}

    /** @implementation-alias array_buffer_view_current */
    public function current(): int {}
}

class Int32Array implements ArrayAccess, Iterator {
    /** @implementation-alias array_buffer_view_ctor */
    public function __construct(ArrayBuffer $buffer) {}

    /** @implementation-alias array_buffer_view_wakeup */
    public function __wakeup(): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_get
     */
    public function offsetGet($offset): int {}

    /**
     * @param int $offset
     * @param int $value
     * @implementation-alias array_buffer_view_offset_set
     */
    public function offsetSet($offset, $value): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_exists
     */
    public function offsetExists($offset): bool {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_unset
     */
    public function offsetUnset($offset): void {}

    /** @implementation-alias array_buffer_view_rewind */
    public function rewind(): void {}

    /** @implementation-alias array_buffer_view_next */
    public function next(): void {}

    /** @implementation-alias array_buffer_view_valid */
    public function valid(): bool {}

    /** @implementation-alias array_buffer_view_key */
    public function key(): int {}

    /** @implementation-alias array_buffer_view_current */
    public function current(): int {}
}

class UInt32Array implements ArrayAccess, Iterator {
    /** @implementation-alias array_buffer_view_ctor */
    public function __construct(ArrayBuffer $buffer) {}

    /** @implementation-alias array_buffer_view_wakeup */
    public function __wakeup(): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_get
     */
    public function offsetGet($offset): int {}

    /**
     * @param int $offset
     * @param int|float $value
     * @implementation-alias array_buffer_view_offset_set
     */
    public function offsetSet($offset, $value): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_exists
     */
    public function offsetExists($offset): bool {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_unset
     */
    public function offsetUnset($offset): void {}

    /** @implementation-alias array_buffer_view_rewind */
    public function rewind(): void {}

    /** @implementation-alias array_buffer_view_next */
    public function next(): void {}

    /** @implementation-alias array_buffer_view_valid */
    public function valid(): bool {}

    /** @implementation-alias array_buffer_view_key */
    public function key(): int {}

    /** @implementation-alias array_buffer_view_current */
    public function current(): int|float {}
}

class FloatArray implements ArrayAccess, Iterator {
    /** @implementation-alias array_buffer_view_ctor */
    public function __construct(ArrayBuffer $buffer) {}

    /** @implementation-alias array_buffer_view_wakeup */
    public function __wakeup(): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_get
     */
    public function offsetGet($offset): int {}

    /**
     * @param int $offset
     * @param float $value
     * @implementation-alias array_buffer_view_offset_set
     */
    public function offsetSet($offset, $value): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_exists
     */
    public function offsetExists($offset): bool {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_unset
     */
    public function offsetUnset($offset): void {}

    /** @implementation-alias array_buffer_view_rewind */
    public function rewind(): void {}

    /** @implementation-alias array_buffer_view_next */
    public function next(): void {}

    /** @implementation-alias array_buffer_view_valid */
    public function valid(): bool {}

    /** @implementation-alias array_buffer_view_key */
    public function key(): int {}

    /** @implementation-alias array_buffer_view_current */
    public function current(): float {}
}

class DoubleArray implements ArrayAccess, Iterator {
    /** @implementation-alias array_buffer_view_ctor */
    public function __construct(ArrayBuffer $buffer) {}

    /** @implementation-alias array_buffer_view_wakeup */
    public function __wakeup(): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_get
     */
    public function offsetGet($offset): int {}

    /**
     * @param int $offset
     * @param float $value
     * @implementation-alias array_buffer_view_offset_set
     */
    public function offsetSet($offset, $value): void {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_exists
     */
    public function offsetExists($offset): bool {}

    /**
     * @param int $offset
     * @implementation-alias array_buffer_view_offset_unset
     */
    public function offsetUnset($offset): void {}

    /** @implementation-alias array_buffer_view_rewind */
    public function rewind(): void {}

    /** @implementation-alias array_buffer_view_next */
    public function next(): void {}

    /** @implementation-alias array_buffer_view_valid */
    public function valid(): bool {}

    /** @implementation-alias array_buffer_view_key */
    public function key(): int {}

    /** @implementation-alias array_buffer_view_current */
    public function current(): float {}
}
