/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group                                |
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

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_buffer.h"

#include "zend_exceptions.h"
#include "zend_interfaces.h"

#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"

#if defined(PHP_WIN32)
# include "win32/php_stdint.h"
#elif defined(HAVE_STDINT_H)
# include <stdint.h>
#endif

zend_module_entry buffer_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"buffer",
	NULL,
	PHP_MINIT(buffer),
	NULL,
	NULL,	
	NULL,
	NULL,
#if ZEND_MODULE_API_NO >= 20010901
	"0.1",
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_BUFFER
ZEND_GET_MODULE(buffer)
#endif

typedef struct _buffer_object {
	zend_object std;

	void *buffer;
	size_t length;
} buffer_object;

typedef enum _buffer_view_type {
	buffer_view_int8,
	buffer_view_uint8,
	buffer_view_int16,
	buffer_view_uint16,
	buffer_view_int32,
	buffer_view_uint32,
	buffer_view_float,
	buffer_view_double
} buffer_view_type;

typedef struct _buffer_view_object {
	zend_object std;

	zval *buffer_zval;

	union {
		int8_t   *as_int8;
		uint8_t  *as_uint8;
		int16_t  *as_int16;
		uint16_t *as_uint16;
		int32_t  *as_int32;
		uint32_t *as_uint32;
		float    *as_float;
		double   *as_double;
	} buf;

	size_t offset;
	size_t length;

	/* For Iterator methods */
	size_t current_offset;

	buffer_view_type type;
} buffer_view_object;

typedef struct _buffer_view_iterator {
	zend_object_iterator intern;
	buffer_view_object *view;
	size_t offset;
	zval *current;
} buffer_view_iterator;

zend_class_entry *array_buffer_ce;

zend_class_entry *int8_array_ce;
zend_class_entry *uint8_array_ce;
zend_class_entry *int16_array_ce;
zend_class_entry *uint16_array_ce;
zend_class_entry *int32_array_ce;
zend_class_entry *uint32_array_ce;
zend_class_entry *float_array_ce;
zend_class_entry *double_array_ce;

zend_object_handlers array_buffer_handlers;
zend_object_handlers array_buffer_view_handlers;

static long get_long_from_zval(zval *zv)
{
	if (Z_TYPE_P(zv)) {
		return Z_LVAL_P(zv);
	} else {
		long lval;
		Z_ADDREF_P(zv);
		convert_to_long_ex(&zv);
		lval = Z_LVAL_P(zv);
		zval_ptr_dtor(&zv);
		return lval;
	}
}

static void array_buffer_free_object_storage(buffer_object *intern TSRMLS_DC)
{
	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if (intern->buffer) {
		efree(intern->buffer);
	}

	efree(intern);
}

zend_object_value array_buffer_create_object(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;

	buffer_object *intern = emalloc(sizeof(buffer_object));
	memset(intern, 0, sizeof(buffer_object));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	retval.handle = zend_objects_store_put(intern,
		(zend_objects_store_dtor_t) zend_objects_destroy_object,
		(zend_objects_free_object_storage_t) array_buffer_free_object_storage,
		NULL TSRMLS_CC
	);
	retval.handlers = &array_buffer_handlers;

	return retval;
}

static zend_object_value array_buffer_clone(zval *object TSRMLS_DC)
{
	buffer_object *old_object = zend_object_store_get_object(object TSRMLS_CC);
	zend_object_value new_object_val = array_buffer_create_object(Z_OBJCE_P(object) TSRMLS_CC);
	buffer_object *new_object = zend_object_store_get_object_by_handle(
		new_object_val.handle TSRMLS_CC
	);

	zend_objects_clone_members(
		&new_object->std, new_object_val,
		&old_object->std, Z_OBJ_HANDLE_P(object) TSRMLS_CC
	);

	new_object->buffer = old_object->buffer;
	new_object->length = old_object->length;

	if (old_object->buffer) {
		new_object->buffer = emalloc(old_object->length);
		memcpy(new_object->buffer, old_object->buffer, old_object->length);
	}

	return new_object_val;
}

PHP_METHOD(ArrayBuffer, __construct)
{
	buffer_object *intern;
	long length;
	zend_error_handling error_handling;

	zend_replace_error_handling(EH_THROW, NULL, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &length) == FAILURE) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	if (length <= 0) {
		zend_throw_exception(NULL, "Buffer length must be positive", 0 TSRMLS_CC);
		return;
	}

	intern = zend_object_store_get_object(getThis() TSRMLS_CC);

	intern->buffer = emalloc(length);
	intern->length = length;

	memset(intern->buffer, 0, length);
}

PHP_METHOD(ArrayBuffer, serialize)
{
	buffer_object *intern;
	smart_str buf = {0};
	php_serialize_data_t var_hash;
	zval zv;
	zval *zv_ptr = &zv;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	intern = zend_object_store_get_object(getThis() TSRMLS_CC);
	if (!intern->buffer) {
		return;
	}

	PHP_VAR_SERIALIZE_INIT(var_hash);

	INIT_PZVAL(zv_ptr);

	/* Serialize buffer as string */
	ZVAL_STRINGL(zv_ptr, (char *) intern->buffer, (int) intern->length, 0);
	php_var_serialize(&buf, &zv_ptr, &var_hash TSRMLS_CC);

	/* Serialize properties as array */
	Z_ARRVAL_P(zv_ptr) = zend_std_get_properties(getThis() TSRMLS_CC);
	Z_TYPE_P(zv_ptr) = IS_ARRAY;
	php_var_serialize(&buf, &zv_ptr, &var_hash TSRMLS_CC);

	PHP_VAR_SERIALIZE_DESTROY(var_hash);

	if (buf.c) {
		RETURN_STRINGL(buf.c, buf.len, 0);
	}
}

PHP_METHOD(ArrayBuffer, unserialize)
{
	buffer_object *intern;
	char *str;
	int str_len;
	php_unserialize_data_t var_hash;
	const unsigned char *p, *max;
	zval zv, *zv_ptr = &zv;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &str_len) == FAILURE) {
		return;
	}

	intern = zend_object_store_get_object(getThis() TSRMLS_CC);

	if (intern->buffer) {
		zend_throw_exception(
			NULL, "Cannot call unserialize() on an already constructed object", 0 TSRMLS_CC
		);
		return;
	}

	PHP_VAR_UNSERIALIZE_INIT(var_hash);

	p = (unsigned char *) str;
	max = (unsigned char *) str + str_len;

	INIT_ZVAL(zv);
	if (!php_var_unserialize(&zv_ptr, &p, max, &var_hash TSRMLS_CC)
	    || Z_TYPE_P(zv_ptr) != IS_STRING || Z_STRLEN_P(zv_ptr) == 0) {
		zend_throw_exception(NULL, "Could not unserialize buffer", 0 TSRMLS_CC);
		goto exit;
	}

	intern->buffer = Z_STRVAL_P(zv_ptr);
	intern->length = Z_STRLEN_P(zv_ptr);

	INIT_ZVAL(zv);
	if (!php_var_unserialize(&zv_ptr, &p, max, &var_hash TSRMLS_CC)
	    || Z_TYPE_P(zv_ptr) != IS_ARRAY) {
		zend_throw_exception(NULL, "Could not unserialize properties", 0 TSRMLS_CC);
		goto exit;
	}

	if (zend_hash_num_elements(Z_ARRVAL_P(zv_ptr)) != 0) {
		zend_hash_copy(
			zend_std_get_properties(getThis() TSRMLS_CC), Z_ARRVAL_P(zv_ptr),
			(copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval *)
		);
	}

exit:
	zval_dtor(zv_ptr);
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
}

static void array_buffer_view_free_object_storage(buffer_view_object *intern TSRMLS_DC)
{
	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if (intern->buffer_zval) {
		zval_ptr_dtor(&intern->buffer_zval);
	}

	efree(intern);
}

zend_object_value array_buffer_view_create_object(zend_class_entry *class_type TSRMLS_DC)
{
	zend_object_value retval;

	buffer_view_object *intern = emalloc(sizeof(buffer_view_object));
	memset(intern, 0, sizeof(buffer_view_object));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	{
		zend_class_entry *base_class_type = class_type;

		while (base_class_type->parent) {
			base_class_type = base_class_type->parent;
		}

		if (base_class_type == int8_array_ce) {
			intern->type = buffer_view_int8;
		} else if (base_class_type == uint8_array_ce) {
			intern->type = buffer_view_uint8;
		} else if (base_class_type == int16_array_ce) {
			intern->type = buffer_view_int16;
		} else if (base_class_type == uint16_array_ce) {
			intern->type = buffer_view_uint16;
		} else if (base_class_type == int32_array_ce) {
			intern->type = buffer_view_int32;
		} else if (base_class_type == uint32_array_ce) {
			intern->type = buffer_view_uint32;
		} else if (base_class_type == float_array_ce) {
			intern->type = buffer_view_float;
		} else if (base_class_type == double_array_ce) {
			intern->type = buffer_view_double;
		} else {
			/* Should never happen */
			zend_error(E_ERROR, "Buffer view does not have a valid base class");
		}
	}

	retval.handle = zend_objects_store_put(intern,
		(zend_objects_store_dtor_t) zend_objects_destroy_object,
		(zend_objects_free_object_storage_t) array_buffer_view_free_object_storage,
		NULL TSRMLS_CC
	);
	retval.handlers = &array_buffer_view_handlers;

	return retval;
}

static zend_object_value array_buffer_view_clone(zval *object TSRMLS_DC)
{
	buffer_view_object *old_object = zend_object_store_get_object(object TSRMLS_CC);
	zend_object_value new_object_val = array_buffer_view_create_object(
		Z_OBJCE_P(object) TSRMLS_CC
	);
	buffer_view_object *new_object = zend_object_store_get_object_by_handle(
		new_object_val.handle TSRMLS_CC
	);

	zend_objects_clone_members(
		&new_object->std, new_object_val,
		&old_object->std, Z_OBJ_HANDLE_P(object) TSRMLS_CC
	);

	new_object->buffer_zval = old_object->buffer_zval;
	if (new_object->buffer_zval) {
		Z_ADDREF_P(new_object->buffer_zval);
	}

	new_object->buf.as_int8 = old_object->buf.as_int8;
	new_object->offset = old_object->offset;
	new_object->length = old_object->length;
	new_object->type   = old_object->type;

	return new_object_val;
}

size_t buffer_view_get_bytes_per_element(buffer_view_object *intern)
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

zval *buffer_view_offset_get(buffer_view_object *intern, size_t offset)
{
	zval *retval;
	MAKE_STD_ZVAL(retval);

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
			if (value <= LONG_MAX) {
				ZVAL_LONG(retval, value);
			} else {
				ZVAL_DOUBLE(retval, value);
			}
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

	return retval;
}

void buffer_view_offset_set(buffer_view_object *intern, long offset, zval *value)
{
	if (intern->type == buffer_view_float || intern->type == buffer_view_double) {
		Z_ADDREF_P(value);
		convert_to_double_ex(&value);

		if (intern->type == buffer_view_float) {
			intern->buf.as_float[offset] = Z_DVAL_P(value);
		} else {
			intern->buf.as_double[offset] = Z_DVAL_P(value);
		}

		zval_ptr_dtor(&value);
	} else {
		long lval = get_long_from_zval(value);

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

static zval *array_buffer_view_read_dimension(zval *object, zval *zv_offset, int type TSRMLS_DC)
{
	buffer_view_object *intern = zend_object_store_get_object(object TSRMLS_CC);
	zval *retval;
	long offset;

	if (intern->std.ce->parent) {
		return zend_get_std_object_handlers()->read_dimension(object, zv_offset, type TSRMLS_CC);
	}
	
	if (!zv_offset) {
		zend_throw_exception(NULL, "Cannot append to a typed array", 0 TSRMLS_CC);
		return NULL;
	}

	offset = get_long_from_zval(zv_offset);
	if (offset < 0 || offset >= intern->length) {
		zend_throw_exception(NULL, "Offset is outside the buffer range", 0 TSRMLS_CC);
		return NULL;
	}

	retval = buffer_view_offset_get(intern, offset);
	Z_DELREF_P(retval); /* Refcount should be 0 if not referenced from ext / engine */
	return retval;
}

static void array_buffer_view_write_dimension(zval *object, zval *zv_offset, zval *value TSRMLS_DC)
{
	buffer_view_object *intern = zend_object_store_get_object(object TSRMLS_CC);
	long offset;
	
	if (intern->std.ce->parent) {
		zend_get_std_object_handlers()->write_dimension(object, zv_offset, value TSRMLS_CC);
		return;
	}
	
	if (!zv_offset) {
		zend_throw_exception(NULL, "Cannot append to a typed array", 0 TSRMLS_CC);
		return;
	}
	
	offset = get_long_from_zval(zv_offset);
	if (offset < 0 || offset >= intern->length) {
		zend_throw_exception(NULL, "Offset is outside the buffer range", 0 TSRMLS_CC);
		return;
	}

	buffer_view_offset_set(intern, offset, value);
}

static int array_buffer_view_has_dimension(zval *object, zval *zv_offset, int check_empty TSRMLS_DC)
{
	buffer_view_object *intern = zend_object_store_get_object(object TSRMLS_CC);
	long offset = get_long_from_zval(zv_offset);
 
	if (intern->std.ce->parent) {
		return zend_get_std_object_handlers()->has_dimension(object, zv_offset, check_empty TSRMLS_CC);
	}
	
	if (offset < 0 || offset >= intern->length) {
		return 0;
	}

	if (check_empty) {
		int retval;
		zval *value = buffer_view_offset_get(intern, offset);
		retval = zend_is_true(value);
		zval_ptr_dtor(&value);
		return retval;
	}

	return 1;
}

static void array_buffer_view_unset_dimension(zval *object, zval *zv_offset TSRMLS_DC)
{
	buffer_view_object *intern = zend_object_store_get_object(object TSRMLS_CC);

	if (intern->std.ce->parent) {
		zend_get_std_object_handlers()->unset_dimension(object, zv_offset TSRMLS_CC);
		return;
	}
	
	zend_throw_exception(NULL, "Cannot unset offsets in a typed array", 0 TSRMLS_CC);
}

static int array_buffer_view_compare_objects(zval *obj1, zval *obj2 TSRMLS_DC)
{
	buffer_view_object *intern1 = zend_object_store_get_object(obj1 TSRMLS_CC);	
	buffer_view_object *intern2 = zend_object_store_get_object(obj2 TSRMLS_CC);	

	if (memcmp(intern1, intern2, sizeof(buffer_view_object)) == 0) {
		return 0; /* equal */
	} else {
		return 1; /* not orderable */
	}
}

static HashTable *array_buffer_view_get_debug_info(zval *obj, int *is_temp TSRMLS_DC)
{
	buffer_view_object *intern = zend_object_store_get_object(obj TSRMLS_CC);	
	HashTable *props = Z_OBJPROP_P(obj);
	HashTable *ht;
	int i;

	ALLOC_HASHTABLE(ht);
	ZEND_INIT_SYMTABLE_EX(ht, intern->length + zend_hash_num_elements(props), 0);
	zend_hash_copy(ht, props, (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval *));

	*is_temp = 1;

	for (i = 0; i < intern->length; ++i) {
		zval *value = buffer_view_offset_get(intern, i);
		zend_hash_index_update(ht, i, (void *) &value, sizeof(zval *), NULL);
	}
	
	return ht;
}

static HashTable *array_buffer_view_get_properties(zval *obj TSRMLS_DC)
{
	buffer_view_object *intern = zend_object_store_get_object(obj TSRMLS_CC);	
	HashTable *ht = zend_std_get_properties(obj TSRMLS_CC);
	zval *zv;

	if (!intern->buffer_zval) {
		return ht;
	}

	Z_ADDREF_P(intern->buffer_zval);
	zend_hash_update(ht, "buffer", sizeof("buffer"), &intern->buffer_zval, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(zv);
	ZVAL_LONG(zv, intern->offset);
	zend_hash_update(ht, "offset", sizeof("offset"), &zv, sizeof(zval *), NULL);

	MAKE_STD_ZVAL(zv);
	ZVAL_LONG(zv, intern->length);
	zend_hash_update(ht, "length", sizeof("length"), &zv, sizeof(zval *), NULL);
	
	return ht;
}

static void buffer_view_iterator_dtor(zend_object_iterator *intern TSRMLS_DC)
{
	buffer_view_iterator *iter = (buffer_view_iterator *) intern;

	if (iter->current) {
		zval_ptr_dtor(&iter->current);
	}

	zval_ptr_dtor((zval **) &intern->data);
	efree(iter);
}

static int buffer_view_iterator_valid(zend_object_iterator *intern TSRMLS_DC)
{
	buffer_view_iterator *iter = (buffer_view_iterator *) intern;

	return iter->offset < iter->view->length ? SUCCESS : FAILURE;
}

static void buffer_view_iterator_get_current_data(zend_object_iterator *intern, zval ***data TSRMLS_DC)
{
	buffer_view_iterator *iter = (buffer_view_iterator *) intern;

	if (iter->current) {
		zval_ptr_dtor(&iter->current);
	}

	if (iter->offset < iter->view->length) {
		iter->current = buffer_view_offset_get(iter->view, iter->offset);
		*data = &iter->current;
	} else {
		*data = NULL;
	}
}

#if ZEND_MODULE_API_NO >= 20121212
static void buffer_view_iterator_get_current_key(zend_object_iterator *intern, zval *key TSRMLS_DC)
{
	buffer_view_iterator *iter = (buffer_view_iterator *) intern;
	ZVAL_LONG(key, iter->offset);
}
#else
static int buffer_view_iterator_get_current_key(zend_object_iterator *intern, char **str_key, uint *str_key_len, ulong *int_key TSRMLS_DC)
{
	buffer_view_iterator *iter = (buffer_view_iterator *) intern;

	*int_key = (ulong) iter->offset;
	return HASH_KEY_IS_LONG;
}
#endif

static void buffer_view_iterator_move_forward(zend_object_iterator *intern TSRMLS_DC)
{
	buffer_view_iterator *iter = (buffer_view_iterator *) intern;

	iter->offset++;
}

static void buffer_view_iterator_rewind(zend_object_iterator *intern TSRMLS_DC)
{
	buffer_view_iterator *iter = (buffer_view_iterator *) intern;

	iter->offset = 0;
	iter->current = NULL;
}

static zend_object_iterator_funcs buffer_view_iterator_funcs = {
	buffer_view_iterator_dtor,
	buffer_view_iterator_valid,
	buffer_view_iterator_get_current_data,
	buffer_view_iterator_get_current_key,
	buffer_view_iterator_move_forward,
	buffer_view_iterator_rewind
};

zend_object_iterator *buffer_view_get_iterator(zend_class_entry *ce, zval *object, int by_ref TSRMLS_DC)
{
	buffer_view_iterator *iter;

	if (by_ref) {
		zend_throw_exception(NULL, "Cannot iterate buffer view by reference", 0 TSRMLS_CC);
		return NULL;
	}

	iter = emalloc(sizeof(buffer_view_iterator));
	iter->intern.funcs = &buffer_view_iterator_funcs;

	iter->intern.data = object;
	Z_ADDREF_P(object);

	iter->view = zend_object_store_get_object(object TSRMLS_CC);
	iter->offset = 0;
	iter->current = NULL;

	return (zend_object_iterator *) iter;
}

PHP_FUNCTION(array_buffer_view_ctor)
{
	zval *buffer_zval;
	long offset = 0, length = 0;
	buffer_view_object *view_intern;
	buffer_object *buffer_intern;
	zend_error_handling error_handling;

	zend_replace_error_handling(EH_THROW, NULL, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|ll", &buffer_zval, array_buffer_ce, &offset, &length) == FAILURE) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	view_intern = zend_object_store_get_object(getThis() TSRMLS_CC);
	buffer_intern = zend_object_store_get_object(buffer_zval TSRMLS_CC);

	if (offset < 0) {
		zend_throw_exception(NULL, "Offset must be non-negative", 0 TSRMLS_CC);
		return;
	}
	if (offset >= buffer_intern->length) {
		zend_throw_exception(NULL, "Offset has to be smaller than the buffer length", 0 TSRMLS_CC);
		return;
	}
	if (length < 0) {
		zend_throw_exception(NULL, "Length must be positive or zero", 0 TSRMLS_CC);
		return;
	}

	view_intern->offset = offset;
	view_intern->buffer_zval = buffer_zval;
	Z_ADDREF_P(buffer_zval);

	{
		size_t bytes_per_element = buffer_view_get_bytes_per_element(view_intern);
		size_t max_length = (buffer_intern->length - offset) / bytes_per_element;

		if (length == 0) {
			view_intern->length = max_length;
		} else if (length > max_length) {
			zend_throw_exception(NULL, "Length is larger than the buffer", 0 TSRMLS_CC);
			return;
		} else {
			view_intern->length = length;
		}
	}

	view_intern->buf.as_int8 = buffer_intern->buffer;
	view_intern->buf.as_int8 += offset;
}

PHP_FUNCTION(array_buffer_view_wakeup)
{
	buffer_view_object *intern;
	HashTable *props;
	zval **buffer_zv, **offset_zv, **length_zv;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	intern = zend_object_store_get_object(getThis() TSRMLS_CC);

	if (intern->buffer_zval) {
		zend_throw_exception(
			NULL, "Cannot call __wakeup() on an already constructed object", 0 TSRMLS_CC
		);
		return;
	}

	props = zend_std_get_properties(getThis() TSRMLS_CC);

	if (zend_hash_find(props, "buffer", sizeof("buffer"), (void **) &buffer_zv) == SUCCESS
	 && zend_hash_find(props, "offset", sizeof("offset"), (void **) &offset_zv) == SUCCESS
	 && zend_hash_find(props, "length", sizeof("length"), (void **) &length_zv) == SUCCESS
	 && Z_TYPE_PP(buffer_zv) == IS_OBJECT
	 && Z_TYPE_PP(offset_zv) == IS_LONG && Z_LVAL_PP(offset_zv) >= 0
	 && Z_TYPE_PP(length_zv) == IS_LONG && Z_LVAL_PP(length_zv) > 0
	 && instanceof_function(Z_OBJCE_PP(buffer_zv), array_buffer_ce TSRMLS_CC)
	) {
		buffer_object *buffer_intern = zend_object_store_get_object(*buffer_zv TSRMLS_CC);
		size_t offset = Z_LVAL_PP(offset_zv), length = Z_LVAL_PP(length_zv);
		size_t bytes_per_element = buffer_view_get_bytes_per_element(intern);
		size_t max_length = (buffer_intern->length - offset) / bytes_per_element;

		if (offset < buffer_intern->length && length <= max_length) {
			Z_ADDREF_PP(buffer_zv);
			intern->buffer_zval = *buffer_zv;

			intern->offset = offset;
			intern->length = length;

			intern->buf.as_int8 = buffer_intern->buffer;
			intern->buf.as_int8 += offset;

			return;
		}
	}

	zend_throw_exception(
		NULL, "Invalid serialization data", 0 TSRMLS_CC
	);
}

PHP_FUNCTION(array_buffer_view_offset_get)
{
	buffer_view_object *intern;
	long offset;
	zval *retval;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &offset) == FAILURE) {
		return;
	}

	intern = zend_object_store_get_object(getThis() TSRMLS_CC);

	if (offset < 0 || offset >= intern->length) {
		zend_throw_exception(NULL, "Offset is outside the buffer range", 0 TSRMLS_CC);
		return;
	}

	retval = buffer_view_offset_get(intern, offset);
	RETURN_ZVAL(retval, 1, 1);
}

PHP_FUNCTION(array_buffer_view_offset_set)
{
	buffer_view_object *intern;
	long offset;
	zval *value;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lz", &offset, &value) == FAILURE) {
		return;
	}

	intern = zend_object_store_get_object(getThis() TSRMLS_CC);

	if (offset < 0 || offset >= intern->length) {
		zend_throw_exception(NULL, "Offset is outside the buffer range", 0 TSRMLS_CC);
		return;
	}

	buffer_view_offset_set(intern, offset, value);
}

PHP_FUNCTION(array_buffer_view_offset_exists)
{
	buffer_view_object *intern;
	long offset;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &offset) == FAILURE) {
		return;
	}

	intern = zend_object_store_get_object(getThis() TSRMLS_CC);

	RETURN_BOOL(offset < intern->length);
}

PHP_FUNCTION(array_buffer_view_offset_unset)
{
	long offset;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &offset) == FAILURE) {
		return;
	}

	/* I don't think that the unset() operations makes sense on typed arrays. If you want
	 * to zero out an offset just assign 0 to it. */
	zend_throw_exception(NULL, "Cannot unset offsets in a typed array", 0 TSRMLS_CC);
}

PHP_FUNCTION(array_buffer_view_rewind)
{
	buffer_view_object *intern;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	intern = zend_object_store_get_object(getThis() TSRMLS_CC);
	intern->current_offset = 0;
}

PHP_FUNCTION(array_buffer_view_next)
{
	buffer_view_object *intern;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	intern = zend_object_store_get_object(getThis() TSRMLS_CC);
	intern->current_offset++;
}

PHP_FUNCTION(array_buffer_view_valid)
{
	buffer_view_object *intern;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	intern = zend_object_store_get_object(getThis() TSRMLS_CC);
	RETURN_BOOL(intern->current_offset < intern->length);
}

PHP_FUNCTION(array_buffer_view_key)
{
	buffer_view_object *intern;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	intern = zend_object_store_get_object(getThis() TSRMLS_CC);
	RETURN_LONG((long) intern->current_offset);
}

PHP_FUNCTION(array_buffer_view_current)
{
	buffer_view_object *intern;
	zval *value;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	intern = zend_object_store_get_object(getThis() TSRMLS_CC);
	value = buffer_view_offset_get(intern, intern->current_offset);
	RETURN_ZVAL(value, 1, 1);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_buffer_ctor, 0, 0, 1)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_buffer_void, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_buffer_unserialize, 0, 0, 1)
	ZEND_ARG_INFO(0, serialized)
ZEND_END_ARG_INFO()

const zend_function_entry array_buffer_functions[] = {
	PHP_ME(ArrayBuffer, __construct, arginfo_buffer_ctor, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(ArrayBuffer, serialize, arginfo_buffer_void, ZEND_ACC_PUBLIC)
	PHP_ME(ArrayBuffer, unserialize, arginfo_buffer_unserialize, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

ZEND_BEGIN_ARG_INFO_EX(arginfo_buffer_view_ctor, 0, 0, 1)
	ZEND_ARG_INFO(0, buffer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_buffer_view_offset, 0, 0, 1)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_buffer_view_offset_set, 0, 0, 2)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_buffer_view_void, 0, 0, 0)
ZEND_END_ARG_INFO()

const zend_function_entry array_buffer_view_functions[] = {
	PHP_ME_MAPPING(__construct, array_buffer_view_ctor, arginfo_buffer_view_ctor, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME_MAPPING(__wakeup, array_buffer_view_wakeup, arginfo_buffer_view_void, ZEND_ACC_PUBLIC)

	/* ArrayAccess */
	PHP_ME_MAPPING(offsetGet, array_buffer_view_offset_get, arginfo_buffer_view_offset, ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(offsetSet, array_buffer_view_offset_set, arginfo_buffer_view_offset_set, ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(offsetExists, array_buffer_view_offset_exists, arginfo_buffer_view_offset, ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(offsetUnset, array_buffer_view_offset_unset, arginfo_buffer_view_offset, ZEND_ACC_PUBLIC)

	/* Iterator */
	PHP_ME_MAPPING(rewind, array_buffer_view_rewind, arginfo_buffer_view_void, ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(next, array_buffer_view_next, arginfo_buffer_view_void, ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(valid, array_buffer_view_valid, arginfo_buffer_view_void, ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(key, array_buffer_view_key, arginfo_buffer_view_void, ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(current, array_buffer_view_current, arginfo_buffer_view_void, ZEND_ACC_PUBLIC)

	PHP_FE_END
};

PHP_MINIT_FUNCTION(buffer)
{
	zend_class_entry tmp_ce;

	INIT_CLASS_ENTRY(tmp_ce, "ArrayBuffer", array_buffer_functions);
	array_buffer_ce = zend_register_internal_class(&tmp_ce TSRMLS_CC);
	array_buffer_ce->create_object = array_buffer_create_object;
	zend_class_implements(array_buffer_ce TSRMLS_CC, 1, zend_ce_serializable);

#define DEFINE_ARRAY_BUFFER_VIEW_CLASS(class_name, type)                     \
	INIT_CLASS_ENTRY(tmp_ce, #class_name, array_buffer_view_functions);      \
	type##_array_ce = zend_register_internal_class(&tmp_ce TSRMLS_CC);       \
	type##_array_ce->create_object = array_buffer_view_create_object;        \
	type##_array_ce->get_iterator = buffer_view_get_iterator;                \
	type##_array_ce->iterator_funcs.funcs = &buffer_view_iterator_funcs;     \
	zend_class_implements(type##_array_ce TSRMLS_CC, 2,                      \
		zend_ce_arrayaccess, zend_ce_iterator);

	DEFINE_ARRAY_BUFFER_VIEW_CLASS(Int8Array,   int8);
	DEFINE_ARRAY_BUFFER_VIEW_CLASS(UInt8Array,  uint8);
	DEFINE_ARRAY_BUFFER_VIEW_CLASS(Int16Array,  int16);
	DEFINE_ARRAY_BUFFER_VIEW_CLASS(Uint16Array, uint16);
	DEFINE_ARRAY_BUFFER_VIEW_CLASS(Int32Array,  int32);
	DEFINE_ARRAY_BUFFER_VIEW_CLASS(UInt32Array, uint32);
	DEFINE_ARRAY_BUFFER_VIEW_CLASS(FloatArray,  float);
	DEFINE_ARRAY_BUFFER_VIEW_CLASS(DoubleArray, double);

#undef DEFINE_ARRAY_BUFFER_VIEW_CLASS

	memcpy(&array_buffer_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	array_buffer_handlers.clone_obj = array_buffer_clone;

	memcpy(&array_buffer_view_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	array_buffer_view_handlers.clone_obj       = array_buffer_view_clone;
	array_buffer_view_handlers.read_dimension  = array_buffer_view_read_dimension;
	array_buffer_view_handlers.write_dimension = array_buffer_view_write_dimension;
	array_buffer_view_handlers.has_dimension   = array_buffer_view_has_dimension;
	array_buffer_view_handlers.unset_dimension = array_buffer_view_unset_dimension;
	array_buffer_view_handlers.compare_objects = array_buffer_view_compare_objects;
	array_buffer_view_handlers.get_debug_info = array_buffer_view_get_debug_info;
	array_buffer_view_handlers.get_properties  = array_buffer_view_get_properties;

	return SUCCESS;
}
