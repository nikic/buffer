/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 27ace0a062e7121921c7d91c42538d25fbea09ce */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_ArrayBuffer___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, byteLength, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ArrayBuffer___serialize, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ArrayBuffer___unserialize, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_TypedArray___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, buffer, ArrayBuffer, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, byteOffset, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, length, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_TypedArray_offsetGet, 0, 1, MAY_BE_LONG|MAY_BE_DOUBLE)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_TypedArray_offsetSet, 0, 2, IS_VOID, 0)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_TypedArray_offsetExists, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_TypedArray_offsetUnset, 0, 1, IS_VOID, 0)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_TypedArray_getIterator, 0, 0, Iterator, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_TypedArray___serialize arginfo_class_ArrayBuffer___serialize

#define arginfo_class_TypedArray___unserialize arginfo_class_ArrayBuffer___unserialize

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Int8Array_offsetGet, 0, 1, IS_LONG, 0)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

#define arginfo_class_UInt8Array_offsetGet arginfo_class_Int8Array_offsetGet

#define arginfo_class_Int16Array_offsetGet arginfo_class_Int8Array_offsetGet

#define arginfo_class_UInt16Array_offsetGet arginfo_class_Int8Array_offsetGet

#define arginfo_class_Int32Array_offsetGet arginfo_class_Int8Array_offsetGet

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_FloatArray_offsetGet, 0, 1, IS_DOUBLE, 0)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

#define arginfo_class_DoubleArray_offsetGet arginfo_class_FloatArray_offsetGet


ZEND_METHOD(ArrayBuffer, __construct);
ZEND_METHOD(ArrayBuffer, __serialize);
ZEND_METHOD(ArrayBuffer, __unserialize);
ZEND_METHOD(TypedArray, __construct);
ZEND_METHOD(TypedArray, offsetGet);
ZEND_METHOD(TypedArray, offsetSet);
ZEND_METHOD(TypedArray, offsetExists);
ZEND_METHOD(TypedArray, offsetUnset);
ZEND_METHOD(TypedArray, getIterator);
ZEND_METHOD(TypedArray, __serialize);
ZEND_METHOD(TypedArray, __unserialize);


static const zend_function_entry class_ArrayBuffer_methods[] = {
	ZEND_ME(ArrayBuffer, __construct, arginfo_class_ArrayBuffer___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(ArrayBuffer, __serialize, arginfo_class_ArrayBuffer___serialize, ZEND_ACC_PUBLIC)
	ZEND_ME(ArrayBuffer, __unserialize, arginfo_class_ArrayBuffer___unserialize, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_TypedArray_methods[] = {
	ZEND_MALIAS(TypedArray, __construct, __construct, arginfo_class_TypedArray___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(TypedArray, offsetGet, arginfo_class_TypedArray_offsetGet, ZEND_ACC_PUBLIC)
	ZEND_ME(TypedArray, offsetSet, arginfo_class_TypedArray_offsetSet, ZEND_ACC_PUBLIC)
	ZEND_ME(TypedArray, offsetExists, arginfo_class_TypedArray_offsetExists, ZEND_ACC_PUBLIC)
	ZEND_ME(TypedArray, offsetUnset, arginfo_class_TypedArray_offsetUnset, ZEND_ACC_PUBLIC)
	ZEND_ME(TypedArray, getIterator, arginfo_class_TypedArray_getIterator, ZEND_ACC_PUBLIC)
	ZEND_ME(TypedArray, __serialize, arginfo_class_TypedArray___serialize, ZEND_ACC_PUBLIC)
	ZEND_ME(TypedArray, __unserialize, arginfo_class_TypedArray___unserialize, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_Int8Array_methods[] = {
	ZEND_MALIAS(TypedArray, offsetGet, offsetGet, arginfo_class_Int8Array_offsetGet, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_UInt8Array_methods[] = {
	ZEND_MALIAS(TypedArray, offsetGet, offsetGet, arginfo_class_UInt8Array_offsetGet, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_Int16Array_methods[] = {
	ZEND_MALIAS(TypedArray, offsetGet, offsetGet, arginfo_class_Int16Array_offsetGet, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_UInt16Array_methods[] = {
	ZEND_MALIAS(TypedArray, offsetGet, offsetGet, arginfo_class_UInt16Array_offsetGet, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_Int32Array_methods[] = {
	ZEND_MALIAS(TypedArray, offsetGet, offsetGet, arginfo_class_Int32Array_offsetGet, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_UInt32Array_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_FloatArray_methods[] = {
	ZEND_MALIAS(TypedArray, offsetGet, offsetGet, arginfo_class_FloatArray_offsetGet, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};


static const zend_function_entry class_DoubleArray_methods[] = {
	ZEND_MALIAS(TypedArray, offsetGet, offsetGet, arginfo_class_DoubleArray_offsetGet, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_ArrayBuffer(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ArrayBuffer", class_ArrayBuffer_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}

static zend_class_entry *register_class_TypedArray(zend_class_entry *class_entry_ArrayAccess, zend_class_entry *class_entry_IteratorAggregate)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "TypedArray", class_TypedArray_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_ABSTRACT|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 2, class_entry_ArrayAccess, class_entry_IteratorAggregate);

	return class_entry;
}

static zend_class_entry *register_class_Int8Array(zend_class_entry *class_entry_TypedArray)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Int8Array", class_Int8Array_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_TypedArray);
	class_entry->ce_flags |= ZEND_ACC_FINAL;

	return class_entry;
}

static zend_class_entry *register_class_UInt8Array(zend_class_entry *class_entry_TypedArray)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "UInt8Array", class_UInt8Array_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_TypedArray);
	class_entry->ce_flags |= ZEND_ACC_FINAL;

	return class_entry;
}

static zend_class_entry *register_class_Int16Array(zend_class_entry *class_entry_TypedArray)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Int16Array", class_Int16Array_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_TypedArray);
	class_entry->ce_flags |= ZEND_ACC_FINAL;

	return class_entry;
}

static zend_class_entry *register_class_UInt16Array(zend_class_entry *class_entry_TypedArray)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "UInt16Array", class_UInt16Array_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_TypedArray);
	class_entry->ce_flags |= ZEND_ACC_FINAL;

	return class_entry;
}

static zend_class_entry *register_class_Int32Array(zend_class_entry *class_entry_TypedArray)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Int32Array", class_Int32Array_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_TypedArray);
	class_entry->ce_flags |= ZEND_ACC_FINAL;

	return class_entry;
}

static zend_class_entry *register_class_UInt32Array(zend_class_entry *class_entry_TypedArray)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "UInt32Array", class_UInt32Array_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_TypedArray);
	class_entry->ce_flags |= ZEND_ACC_FINAL;

	return class_entry;
}

static zend_class_entry *register_class_FloatArray(zend_class_entry *class_entry_TypedArray)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "FloatArray", class_FloatArray_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_TypedArray);
	class_entry->ce_flags |= ZEND_ACC_FINAL;

	return class_entry;
}

static zend_class_entry *register_class_DoubleArray(zend_class_entry *class_entry_TypedArray)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "DoubleArray", class_DoubleArray_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_TypedArray);
	class_entry->ce_flags |= ZEND_ACC_FINAL;

	return class_entry;
}
