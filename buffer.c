/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2022 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Nikita Popov (nikic@php.net)                                 |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_globals.h"
#include "ext/standard/info.h"
#include "ext/standard/basic_functions.h"
#include "php_buffer.h"

#include "zend_exceptions.h"
#include "zend_interfaces.h"

#include "ext/standard/php_var.h"
#include "zend_smart_str_public.h"

#include <stdint.h>

#include "buffer_arginfo.h"

#ifdef COMPILE_DL_BUFFER
ZEND_GET_MODULE(buffer)
#endif


static zend_class_entry *array_buffer_ce;
static zend_class_entry *typed_array_ce;

static zend_class_entry *int8_array_ce;
static zend_class_entry *uint8_array_ce;
static zend_class_entry *int16_array_ce;
static zend_class_entry *uint16_array_ce;
static zend_class_entry *int32_array_ce;
static zend_class_entry *uint32_array_ce;
static zend_class_entry *float_array_ce;
static zend_class_entry *double_array_ce;

static zend_object_handlers array_buffer_handlers;
static zend_object_handlers array_buffer_view_handlers;

static void array_buffer_free(zend_object *object)
{
	buffer_object *intern = php_buffer_fetch_object(object);

	if (intern->buffer) {
		efree(intern->buffer);
	}

	zend_object_std_dtor(&intern->std);
}

static zend_object *array_buffer_create_object(zend_class_entry *class_type)
{
	buffer_object *intern = zend_object_alloc(sizeof(buffer_object), class_type);
	intern->buffer = NULL;

	zend_object_std_init(&intern->std, class_type);
	intern->std.handlers = &array_buffer_handlers;

	return &intern->std;
}

static zend_object *array_buffer_clone(zend_object *object)
{
	buffer_object *old_object = php_buffer_fetch_object(object);
	zend_object *znew_object = array_buffer_create_object(old_object->std.ce);
	buffer_object *new_object = php_buffer_fetch_object(znew_object);

	zend_objects_clone_members(znew_object, &old_object->std);

	new_object->buffer = old_object->buffer;
	new_object->length = old_object->length;

	if (old_object->buffer) {
		new_object->buffer = emalloc(old_object->length);
		memcpy(new_object->buffer, old_object->buffer, old_object->length);
	}

	return &new_object->std;
}

PHP_METHOD(ArrayBuffer, __construct)
{
	zend_long length;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(length)
	ZEND_PARSE_PARAMETERS_END();

	if (length <= 0) {
		zend_argument_value_error(1, "must be greater than 0");
		RETURN_THROWS();
	}

	buffer_object *intern = Z_BUFFER_OBJ_P(getThis());

	intern->buffer = emalloc(length);
	intern->length = length;

	memset(intern->buffer, 0, length);
}

PHP_METHOD(ArrayBuffer, __serialize)
{
	ZEND_PARSE_PARAMETERS_NONE();

	buffer_object *intern = Z_BUFFER_OBJ_P(getThis());
	array_init(return_value);
	add_assoc_stringl(return_value, "data", intern->buffer, intern->length);
}

PHP_METHOD(ArrayBuffer, __unserialize)
{
	HashTable *ht;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ARRAY_HT(ht)
	ZEND_PARSE_PARAMETERS_END();

	zval *data = zend_hash_str_find(ht, "data", strlen("data"));
	if (!data) {
		zend_throw_exception(NULL, "Could not unserialize buffer: Missing \"data\" entry", 0);
		return;
	}

	if (Z_TYPE_P(data) != IS_STRING) {
		zend_throw_exception(NULL, "Could not unserialize buffer: not a string", 0);
		return;
	}

	if (Z_STRLEN_P(data) == 0) {
		zend_throw_exception(NULL, "Could not unserialize buffer: empty string", 0);
		return;
	}

	buffer_object *intern = Z_BUFFER_OBJ_P(getThis());
	intern->length = Z_STRLEN_P(data);
	intern->buffer = emalloc(intern->length);
	memcpy(intern->buffer, Z_STRVAL_P(data), intern->length);
}

static void array_buffer_view_free(zend_object *obj)
{
	buffer_view_object *intern = php_buffer_view_fetch_object(obj);

	zend_object_std_dtor(&intern->std);

	if (!Z_ISUNDEF(intern->buffer_zval)) {
		zval_ptr_dtor(&intern->buffer_zval);
	}
}

static zend_object *array_buffer_view_create_object(zend_class_entry *class_type)
{
	buffer_view_object *intern = zend_object_alloc(sizeof(buffer_view_object), class_type);
	ZVAL_UNDEF(&intern->buffer_zval);
	intern->offset = 0;
	intern->length = 0;
	intern->current_offset = 0;

	zend_object_std_init(&intern->std, class_type);
	intern->std.handlers = &array_buffer_view_handlers;

	if (class_type == int8_array_ce) {
		intern->type = buffer_view_int8;
	} else if (class_type == uint8_array_ce) {
		intern->type = buffer_view_uint8;
	} else if (class_type == int16_array_ce) {
		intern->type = buffer_view_int16;
	} else if (class_type == uint16_array_ce) {
		intern->type = buffer_view_uint16;
	} else if (class_type == int32_array_ce) {
		intern->type = buffer_view_int32;
	} else if (class_type == uint32_array_ce) {
		intern->type = buffer_view_uint32;
	} else if (class_type == float_array_ce) {
		intern->type = buffer_view_float;
	} else if (class_type == double_array_ce) {
		intern->type = buffer_view_double;
	} else {
		zend_error(E_ERROR, "Trying to instantiate an invalid TypedArray extension");
	}

	return &intern->std;
}

static zend_object *array_buffer_view_clone(zend_object *object)
{
	buffer_view_object *old_object = php_buffer_view_fetch_object(object);
	zend_object *znew_object = array_buffer_view_create_object(
		old_object->std.ce);
	buffer_view_object *new_object = php_buffer_view_fetch_object(znew_object);

	zend_objects_clone_members(znew_object, &old_object->std);

	new_object->buffer_zval = old_object->buffer_zval;
	if (!Z_ISUNDEF(new_object->buffer_zval)) {
                ZVAL_COPY(&new_object->buffer_zval, &old_object->buffer_zval);
	}

	new_object->buf.as_int8 = old_object->buf.as_int8;
	new_object->offset = old_object->offset;
	new_object->length = old_object->length;
	new_object->type   = old_object->type;

	return &new_object->std;
}

static size_t buffer_view_get_bytes_per_element(buffer_view_object *intern)
{
	switch (intern->type)
	{
		case buffer_view_int8:
		case buffer_view_uint8:
			return 1;
		case buffer_view_int16:
		case buffer_view_uint16:
			return 2;
		case buffer_view_int32:
		case buffer_view_uint32:
		case buffer_view_float:
			return 4;
		case buffer_view_double:
			return 8;
		default:
			/* Should never happen */
			zend_error_noreturn(E_ERROR, "Invalid buffer view type");
	}
}

static void buffer_view_offset_get(buffer_view_object *intern, size_t offset, zval *retval)
{
	switch (intern->type) {
		case buffer_view_int8:
			ZVAL_LONG(retval, intern->buf.as_int8[offset]); break;
		case buffer_view_uint8:
			ZVAL_LONG(retval, intern->buf.as_uint8[offset]); break;
		case buffer_view_int16:
			ZVAL_LONG(retval, intern->buf.as_int16[offset]); break;
		case buffer_view_uint16:
			ZVAL_LONG(retval, intern->buf.as_uint16[offset]); break;
		case buffer_view_int32:
			ZVAL_LONG(retval, intern->buf.as_int32[offset]); break;
		case buffer_view_uint32: {
			uint32_t value = intern->buf.as_uint32[offset];
#if SIZEOF_ZEND_LONG == 8
			ZVAL_LONG(retval, value);
#else
			if (value <= ZEND_LONG_MAX) {
				ZVAL_LONG(retval, value);
			} else {
				ZVAL_DOUBLE(retval, value);
			}
#endif
			break;
		}
		case buffer_view_float:
			ZVAL_DOUBLE(retval, intern->buf.as_float[offset]); break;
		case buffer_view_double:
			ZVAL_DOUBLE(retval, intern->buf.as_double[offset]); break;
		default:
			/* Should never happen */
			zend_error_noreturn(E_ERROR, "Invalid buffer view type");
	}
}

void buffer_view_offset_set(buffer_view_object *intern, zend_long offset, zval *value)
{
	if (intern->type == buffer_view_float || intern->type == buffer_view_double) {
		if (intern->type == buffer_view_float) {
			intern->buf.as_float[offset] = zval_get_double(value);
		} else {
			intern->buf.as_double[offset] = zval_get_double(value);
		}

		zval_ptr_dtor(value);
	} else {
		zend_long lval = zval_get_long(value);

		switch (intern->type) {
			case buffer_view_int8:
				intern->buf.as_int8[offset] = lval; break;
			case buffer_view_uint8:
				intern->buf.as_uint8[offset] = lval; break;
			case buffer_view_int16:
				intern->buf.as_int16[offset] = lval; break;
			case buffer_view_uint16:
				intern->buf.as_uint16[offset] = lval; break;
			case buffer_view_int32:
				intern->buf.as_int32[offset] = lval; break;
			case buffer_view_uint32:
				intern->buf.as_uint32[offset] = lval; break;
			default:
				/* Should never happen */
				zend_error(E_ERROR, "Invalid buffer view type");
		}
	}
}

static zval *array_buffer_view_read_dimension(
		zend_object *object, zval *zv_offset, int type, zval *retval)
{
	buffer_view_object *intern = php_buffer_view_fetch_object(object);
	zend_long offset;

	if (intern->std.ce->parent) {
		return zend_get_std_object_handlers()->read_dimension(object, zv_offset, type, retval);
	}
	
	if (!zv_offset) {
		zend_throw_exception(NULL, "Cannot append to a typed array", 0);
		return NULL;
	}

	offset = zval_get_long(zv_offset);
	if (offset < 0 || offset >= intern->length) {
		zend_throw_exception(NULL, "Offset is outside the buffer range", 0);
		return NULL;
	}

	buffer_view_offset_get(intern, offset, retval);
        return retval;
}

static void array_buffer_view_write_dimension(
		zend_object *object, zval *zv_offset, zval *value)
{
	buffer_view_object *intern = php_buffer_view_fetch_object(object);
	zend_long offset;
	
	if (intern->std.ce->parent) {
		zend_get_std_object_handlers()->write_dimension(object, zv_offset, value);
		return;
	}
	
	if (!zv_offset) {
		zend_throw_exception(NULL, "Cannot append to a typed array", 0);
		return;
	}
	
	offset = zval_get_long(zv_offset);
	if (offset < 0 || offset >= intern->length) {
		zend_throw_exception(NULL, "Offset is outside the buffer range", 0);
		return;
	}

	buffer_view_offset_set(intern, offset, value);
}

static int array_buffer_view_has_dimension(
		zend_object *object, zval *zv_offset, int check_empty)
{
	buffer_view_object *intern = php_buffer_view_fetch_object(object);
	zend_long offset = zval_get_long(zv_offset);
 
	if (intern->std.ce->parent) {
		return zend_get_std_object_handlers()->has_dimension(object, zv_offset, check_empty);
	}
	
	if (offset < 0 || offset >= intern->length) {
		return 0;
	}

	if (check_empty) {
		zval value;
		buffer_view_offset_get(intern, offset, &value);
		return zend_is_true(&value);
	}

	return 1;
}

static void array_buffer_view_unset_dimension(zend_object *object, zval *zv_offset)
{
	buffer_view_object *intern = php_buffer_view_fetch_object(object);

	if (intern->std.ce->parent) {
		zend_get_std_object_handlers()->unset_dimension(object, zv_offset);
		return;
	}
	
	zend_throw_exception(NULL, "Cannot unset offsets in a typed array", 0);
}

static int array_buffer_view_compare(zval *obj1, zval *obj2)
{
	ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

	buffer_view_object *intern1 = Z_BUFFER_VIEW_OBJ_P(obj1);
	buffer_view_object *intern2 = Z_BUFFER_VIEW_OBJ_P(obj2);
	size_t type_size;

	if (intern1->type != intern2->type) {
		return 1; /* not orderable */
	}
	if (intern1->length != intern2->length) {
		return 1; /* not orderable */
	}

	type_size = buffer_view_get_bytes_per_element(intern1);
	if (memcmp((void*) &intern1->buf.as_uint8[intern1->offset * type_size],
		(void*) &intern2->buf.as_uint8[intern2->offset * type_size],
		type_size *intern1->length)  == 0) {
		return 0; /* equal */
	} else {
		return 1; /* not orderable */
	}
}

static HashTable *array_buffer_view_get_debug_info(zend_object *obj, int *is_temp)
{
	buffer_view_object *intern = php_buffer_view_fetch_object(obj);
	HashTable *ht = zend_new_array(0);
	zval value;

	*is_temp = 1;
	Z_ADDREF(intern->buffer_zval);
	zend_hash_str_add_new(ht, "buffer", sizeof("buffer")-1, &intern->buffer_zval);

	ZVAL_LONG(&value, intern->offset);
	zend_hash_str_add_new(ht, "offset", sizeof("offset")-1, &value);

	ZVAL_LONG(&value, intern->length);
	zend_hash_str_add_new(ht, "length", sizeof("length")-1, &value);

	for (size_t i = 0; i < intern->length; ++i) {
		buffer_view_offset_get(intern, i, &value);
		zend_hash_index_update(ht, i, &value);
	}
	
	return ht;
}

static void buffer_view_iterator_dtor(zend_object_iterator *intern)
{
	buffer_view_iterator *iter = (buffer_view_iterator*) intern;
	zval_ptr_dtor(&iter->intern.data);
	zval_ptr_dtor(&iter->current);
}

static int buffer_view_iterator_valid(zend_object_iterator *intern)
{
	buffer_view_iterator *iter = (buffer_view_iterator*) intern;

	return iter->offset < iter->view->length ? SUCCESS : FAILURE;
}

static zval *buffer_view_iterator_get_current_data(zend_object_iterator *intern)
{
	buffer_view_iterator *iter = (buffer_view_iterator *) intern;
	return &iter->current;
}

static void buffer_view_iterator_get_current_key(zend_object_iterator *intern, zval *key)
{
	buffer_view_iterator *iter = (buffer_view_iterator *) intern;
	ZVAL_LONG(key, iter->offset);
}

static void buffer_view_iterator_move_forward(zend_object_iterator *intern)
{
	buffer_view_iterator *iter = (buffer_view_iterator*) intern;

	iter->offset++;
	buffer_view_offset_get(iter->view, iter->offset, &iter->current);
}

static void buffer_view_iterator_rewind(zend_object_iterator *intern)
{
	buffer_view_iterator *iter = (buffer_view_iterator*) intern;

	iter->offset = 0;
	buffer_view_offset_get(iter->view, iter->offset, &iter->current);
}

const zend_object_iterator_funcs buffer_view_iterator_funcs = {
	buffer_view_iterator_dtor,
	buffer_view_iterator_valid,
	buffer_view_iterator_get_current_data,
	buffer_view_iterator_get_current_key,
	buffer_view_iterator_move_forward,
	buffer_view_iterator_rewind,
	NULL,
	NULL,
};

static zend_object_iterator *buffer_view_get_iterator(zend_class_entry *ce, zval *object, int by_ref)
{
	buffer_view_iterator *iter;

	if (by_ref) {
		zend_throw_exception(NULL, "Cannot iterate buffer view by reference", 0);
		return NULL;
	}

	iter = ecalloc(1, sizeof(buffer_view_iterator));
	zend_iterator_init(&iter->intern);

	Z_ADDREF_P(object);
	ZVAL_OBJ(&iter->intern.data, Z_OBJ_P(object));

	iter->intern.funcs = &buffer_view_iterator_funcs;
	iter->view = Z_BUFFER_VIEW_OBJ_P(object);
	iter->offset = 0;
	buffer_view_offset_get(iter->view, iter->offset, &iter->current);

	return (zend_object_iterator*) iter;
}

PHP_METHOD(TypedArray, __construct)
{
	zval *buffer_zval;
	zend_long offset = 0, length = 0;

	ZEND_PARSE_PARAMETERS_START(1, 3)
		Z_PARAM_OBJECT_OF_CLASS(buffer_zval, array_buffer_ce)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(offset)
		Z_PARAM_LONG(length)
	ZEND_PARSE_PARAMETERS_END();

	buffer_view_object *view_intern = Z_BUFFER_VIEW_OBJ_P(getThis());
	buffer_object *buffer_intern = Z_BUFFER_OBJ_P(buffer_zval);

	if (offset < 0) {
		zend_argument_value_error(2, "must be greater than or equal to 0");
		RETURN_THROWS();
	}
	if (offset >= buffer_intern->length) {
		zend_argument_value_error(2, "must be smaller than the buffer length");
		RETURN_THROWS();
	}
	if (length < 0) {
		zend_argument_value_error(3, "must be greater than or equal to 0");
		RETURN_THROWS();
	}

	view_intern->offset = offset;
	ZVAL_COPY(&view_intern->buffer_zval, buffer_zval);

	{
		size_t bytes_per_element = buffer_view_get_bytes_per_element(view_intern);
		size_t max_length = (buffer_intern->length - offset) / bytes_per_element;

		if (length == 0) {
			view_intern->length = max_length;
		} else if (length > max_length) {
			zend_argument_value_error(3, "must be smaller than the buffer length");
			RETURN_THROWS();
		} else {
			view_intern->length = length;
		}
	}

	view_intern->buf.as_int8 = buffer_intern->buffer;
	view_intern->buf.as_int8 += offset;
}

PHP_METHOD(TypedArray, offsetGet)
{
	zend_long offset;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(offset)
	ZEND_PARSE_PARAMETERS_END();

	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(getThis());

	if (offset < 0 || offset >= intern->length) {
		zend_throw_exception(NULL, "Offset is outside the buffer range", 0);
		return;
	}

	buffer_view_offset_get(intern, offset, return_value);
}

PHP_METHOD(TypedArray, offsetSet)
{
	zend_long offset;
	zval *value;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_LONG(offset)
		Z_PARAM_ZVAL(value)
	ZEND_PARSE_PARAMETERS_END();

	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(getThis());

	if (offset < 0 || offset >= intern->length) {
		zend_throw_exception(NULL, "Offset is outside the buffer range", 0);
		return;
	}

	buffer_view_offset_set(intern, offset, value);
}

PHP_METHOD(TypedArray, offsetExists)
{
	zend_long offset;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(offset)
	ZEND_PARSE_PARAMETERS_END();

	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(getThis());

	RETURN_BOOL(offset < intern->length);
}

PHP_METHOD(TypedArray, offsetUnset)
{
	zend_long offset;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(offset)
	ZEND_PARSE_PARAMETERS_END();

	/* I don't think that the unset() operations makes sense on typed arrays. If you want
	 * to zero out an offset just assign 0 to it. */
	zend_throw_exception(NULL, "Cannot unset offsets in a typed array", 0);
}

PHP_METHOD(TypedArray, rewind)
{
	ZEND_PARSE_PARAMETERS_NONE();

	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(getThis());
	intern->current_offset = 0;
}

PHP_METHOD(TypedArray, next)
{
	ZEND_PARSE_PARAMETERS_NONE();

	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(getThis());
	intern->current_offset++;
}

PHP_METHOD(TypedArray, valid)
{
	ZEND_PARSE_PARAMETERS_NONE();

	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(getThis());
	RETURN_BOOL(intern->current_offset < intern->length);
}

PHP_METHOD(TypedArray, key)
{
	ZEND_PARSE_PARAMETERS_NONE();

	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(getThis());
	RETURN_LONG((long) intern->current_offset);
}

PHP_METHOD(TypedArray, current)
{
	ZEND_PARSE_PARAMETERS_NONE();

	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(getThis());
	buffer_view_offset_get(intern, intern->current_offset, return_value);
}

PHP_METHOD(TypedArray, __serialize)
{
	ZEND_PARSE_PARAMETERS_NONE();

	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(getThis());
	array_init(return_value);

	Z_ADDREF(intern->buffer_zval);
	add_assoc_zval(return_value, "buffer", &intern->buffer_zval);
	add_assoc_long(return_value, "offset", intern->offset);
	add_assoc_long(return_value, "length", intern->length);
}

PHP_METHOD(TypedArray, __unserialize)
{
	HashTable *ht;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_ARRAY_HT(ht)
	ZEND_PARSE_PARAMETERS_END();

	zval *buffer_zv = zend_hash_str_find(ht, "buffer", strlen("buffer"));
	zval *offset_zv = zend_hash_str_find(ht, "offset", strlen("offset"));
	zval *length_zv = zend_hash_str_find(ht, "length", strlen("length"));
	if (!buffer_zv || !offset_zv || !length_zv) {
		zend_throw_exception(NULL,
			"Could not unserialize view: Missing \"buffer\", \"offset\", or \"length\" entry", 0);
		return;
	}

	if (Z_TYPE_P(buffer_zv) != IS_OBJECT
			|| Z_TYPE_P(offset_zv) != IS_LONG
			|| Z_TYPE_P(length_zv) != IS_LONG
			|| !instanceof_function(Z_OBJCE_P(buffer_zv), array_buffer_ce)) {
		zend_throw_exception(NULL, "Could not unserialize view: Incorrect type", 0);
		return;
	}

	if (Z_LVAL_P(offset_zv) < 0 || Z_LVAL_P(length_zv) <= 0) {
		zend_throw_exception(NULL, "Could not unserialize view: Invalid offset/length", 0);
		return;
	}

	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(getThis());
	buffer_object *buffer_intern = Z_BUFFER_OBJ_P(buffer_zv);
	size_t offset = Z_LVAL_P(offset_zv), length = Z_LVAL_P(length_zv);
	size_t bytes_per_element = buffer_view_get_bytes_per_element(intern);
	size_t max_length = (buffer_intern->length - offset) / bytes_per_element;

	if (offset >= buffer_intern->length || length > max_length) {
		zend_throw_exception(NULL, "Could not unserialize view: Offset/length out of bounds", 0);
		return;
	}

	ZVAL_COPY(&intern->buffer_zval, buffer_zv);

	intern->offset = offset;
	intern->length = length;

	intern->buf.as_int8 = buffer_intern->buffer;
	intern->buf.as_int8 += offset;
}

static PHP_MINIT_FUNCTION(buffer)
{
	array_buffer_ce = register_class_ArrayBuffer();
	array_buffer_ce->create_object = array_buffer_create_object;
	memcpy(&array_buffer_handlers, zend_get_std_object_handlers(), sizeof(array_buffer_handlers));
	array_buffer_handlers.offset = XtOffsetOf(buffer_object, std);
	array_buffer_handlers.free_obj  = array_buffer_free;
	array_buffer_handlers.clone_obj = array_buffer_clone;

	typed_array_ce = register_class_TypedArray(zend_ce_arrayaccess, zend_ce_iterator);
	typed_array_ce->create_object = array_buffer_view_create_object;
	typed_array_ce->get_iterator = buffer_view_get_iterator;

#define DEFINE_ARRAY_BUFFER_VIEW_CLASS(class_name, type) \
	type##_array_ce = register_class_##class_name(typed_array_ce); \

	DEFINE_ARRAY_BUFFER_VIEW_CLASS(Int8Array,   int8);
	DEFINE_ARRAY_BUFFER_VIEW_CLASS(UInt8Array,  uint8);
	DEFINE_ARRAY_BUFFER_VIEW_CLASS(Int16Array,  int16);
	DEFINE_ARRAY_BUFFER_VIEW_CLASS(UInt16Array, uint16);
	DEFINE_ARRAY_BUFFER_VIEW_CLASS(Int32Array,  int32);
	DEFINE_ARRAY_BUFFER_VIEW_CLASS(UInt32Array, uint32);
	DEFINE_ARRAY_BUFFER_VIEW_CLASS(FloatArray,  float);
	DEFINE_ARRAY_BUFFER_VIEW_CLASS(DoubleArray, double);

#undef DEFINE_ARRAY_BUFFER_VIEW_CLASS

	memcpy(&array_buffer_view_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	array_buffer_view_handlers.offset = XtOffsetOf(buffer_view_object, std);
	array_buffer_view_handlers.clone_obj       = array_buffer_view_clone;
	array_buffer_view_handlers.read_dimension  = array_buffer_view_read_dimension;
	array_buffer_view_handlers.write_dimension = array_buffer_view_write_dimension;
	array_buffer_view_handlers.has_dimension   = array_buffer_view_has_dimension;
	array_buffer_view_handlers.unset_dimension = array_buffer_view_unset_dimension;
	array_buffer_view_handlers.compare = array_buffer_view_compare;
	array_buffer_view_handlers.get_debug_info  = array_buffer_view_get_debug_info;
	array_buffer_view_handlers.free_obj        = array_buffer_view_free;

	return SUCCESS;
}

static PHP_MSHUTDOWN_FUNCTION(buffer)
{
	return SUCCESS;
}

PHP_RINIT_FUNCTION(buffer) {
#if defined(COMPILE_DL_BUFFER) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}

PHP_MINFO_FUNCTION(buffer) {
	php_info_print_table_start();
	php_info_print_table_header(2, "array buffer support", "enabled");
	php_info_print_table_row(2, "buffer module version", BUFFER_VERSION);
	php_info_print_table_row(2, "author", "nikic");
	php_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}

zend_module_entry buffer_module_entry = {
	STANDARD_MODULE_HEADER,
	"buffer",
	NULL,
	PHP_MINIT(buffer),
	PHP_MSHUTDOWN(buffer),
	PHP_RINIT(buffer),
	NULL,
	PHP_MINFO(buffer),
	BUFFER_VERSION,
	STANDARD_MODULE_PROPERTIES
};
