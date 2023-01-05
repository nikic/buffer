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

/* $Id$ */

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

#if defined(PHP_WIN32)
# include "win32/php_stdint.h"
#elif defined(HAVE_STDINT_H)
# include <stdint.h>
#endif


#ifdef COMPILE_DL_BUFFER
ZEND_GET_MODULE(buffer)
#endif


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

static void array_buffer_free(zend_object *object)
{
	buffer_object *intern = php_buffer_fetch_object(object);

	if (intern->buffer) {
		efree(intern->buffer);
	}

	zend_object_std_dtor(&intern->std);
}

zend_object *array_buffer_create_object(zend_class_entry *class_type)
{
	zend_object *retval;

	buffer_object *intern = zend_object_alloc(sizeof(buffer_object), class_type);
	intern->buffer = NULL;

	zend_object_std_init(&intern->std, class_type);
	intern->std.handlers = &array_buffer_handlers;

	return &intern->std;
}


static zend_object *array_buffer_clone(zval *object)
{
	buffer_object *old_object = Z_BUFFER_OBJ_P(object);
	zend_object *znew_object = array_buffer_create_object(Z_OBJCE_P(object));
	buffer_object *new_object = php_buffer_fetch_object(znew_object);

	zend_objects_clone_members(znew_object, Z_OBJ_P(object));

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
	buffer_object *intern;
	zend_long length;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "l", &length) == FAILURE) {
		return;
	}

	if (length <= 0) {
		zend_throw_exception(NULL, "Buffer length must be positive", 0);
		return;
	}

	intern = Z_BUFFER_OBJ_P(getThis());

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

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	intern = Z_BUFFER_OBJ_P(getThis());
	if (!intern->buffer) {
		return;
	}

	PHP_VAR_SERIALIZE_INIT(var_hash);

	/* Serialize buffer as string */
        zend_string *zstr = zend_string_init((char*) intern->buffer, intern->length, 0);
	ZVAL_STR(&zv, zstr);
	php_var_serialize(&buf, &zv, &var_hash);
	zend_string_release(zstr);

	/* Serialize properties as array */
	ZVAL_ARR(&zv, zend_std_get_properties(getThis()));
	php_var_serialize(&buf, &zv, &var_hash);

	PHP_VAR_SERIALIZE_DESTROY(var_hash);

        if (buf.s) {
                RETURN_NEW_STR(buf.s);
        }

        RETURN_NULL();
}

PHP_METHOD(ArrayBuffer, unserialize)
{
	buffer_object *intern;
	char *str;
	size_t str_len;
	php_unserialize_data_t var_hash;
	const unsigned char *p, *max;
	zval *zbuf, *ztable;
	zend_string *zstr;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &str, &str_len) == FAILURE) {
		return;
	}
	if (str_len == 0) {
		return;
	}

	intern = Z_BUFFER_OBJ_P(getThis());

	if (intern->buffer) {
		zend_throw_exception(
			NULL, "Cannot call unserialize() on an already constructed object", 0
		);
		return;
	}

	PHP_VAR_UNSERIALIZE_INIT(var_hash);

	p = (unsigned char*) str;
	max = (unsigned char*) str + str_len;

	zbuf = var_tmp_var(&var_hash);
	if (!php_var_unserialize(zbuf, &p, max, &var_hash)) {
		zend_throw_exception(NULL, "Could not unserialize buffer: no data", 0);
		goto exit;
        }
        if(Z_TYPE_P(zbuf) != IS_STRING) {
		zend_throw_exception(NULL, "Could not unserialize buffer: not a string", 0);
		goto exit;
       }

        zstr = zval_get_string(zbuf);
	intern->length = ZSTR_LEN(zstr);
        if(intern->length == 0) {
		zend_throw_exception(NULL, "Could not unserialize buffer: empty string", 0);
		goto exit;
	}
	intern->buffer = emalloc(intern->length);
        memcpy(intern->buffer, &ZSTR_VAL(zstr), intern->length);
        zend_string_release(zstr);

	ztable = var_tmp_var(&var_hash);
	if (!php_var_unserialize(ztable, &p, max, &var_hash)
			|| Z_TYPE_P(ztable) != IS_ARRAY) {
		zend_throw_exception(NULL, "Could not unserialize properties", 0);
		goto exit;
	}

	if (zend_hash_num_elements(Z_ARRVAL_P(ztable)) != 0) {
		zend_hash_copy(
			zend_std_get_properties(getThis()), Z_ARRVAL_P(ztable),
			(copy_ctor_func_t) zval_add_ref
		);
	}

exit:
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
}

static void array_buffer_view_free(zend_object *obj)
{
	buffer_view_object *intern = php_buffer_view_fetch_object(obj);

	zend_object_std_dtor(&intern->std);

	if (!Z_ISUNDEF(intern->buffer_zval)) {
		zval_ptr_dtor(&intern->buffer_zval);
	}
}

zend_object *array_buffer_view_create_object(zend_class_entry *class_type)
{
	zend_object *retval;

	buffer_view_object *intern = emalloc(sizeof(buffer_view_object) +
			                                         zend_object_properties_size(class_type));
	ZVAL_UNDEF(&intern->buffer_zval);
	intern->offset = 0;
	intern->length = 0;
	intern->current_offset = 0;

	zend_object_std_init(&intern->std, class_type);
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

	intern->std.handlers = &array_buffer_view_handlers;

	return &intern->std;
}

static zend_object *array_buffer_view_clone(zval *object)
{
	buffer_view_object *old_object = Z_BUFFER_VIEW_OBJ_P(object);
	zend_object *znew_object = array_buffer_view_create_object(
		Z_OBJCE_P(object)
	);
	buffer_view_object *new_object = php_buffer_view_fetch_object(znew_object);

	zend_objects_clone_members(znew_object, Z_OBJ_P(object));

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

void buffer_view_offset_get(buffer_view_object *intern, size_t offset, zval *retval)
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
}

void buffer_view_offset_set(buffer_view_object *intern, long offset, zval *value)
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

static zval* array_buffer_view_read_dimension(zval *object, zval *zv_offset, int type, zval *retval)
{
	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(object);
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

static void array_buffer_view_write_dimension(zval *object, zval *zv_offset, zval *value)
{
	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(object);
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

static int array_buffer_view_has_dimension(zval *object, zval *zv_offset, int check_empty)
{
	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(object);
	zend_long offset = zval_get_long(zv_offset);
 
	if (intern->std.ce->parent) {
		return zend_get_std_object_handlers()->has_dimension(object, zv_offset, check_empty);
	}
	
	if (offset < 0 || offset >= intern->length) {
		return 0;
	}

	if (check_empty) {
		int retval;
		zval value;
			    buffer_view_offset_get(intern, offset, &value);
		retval = zend_is_true(&value);
		return retval;
	}

	return 1;
}

static void array_buffer_view_unset_dimension(zval *object, zval *zv_offset)
{
	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(object);

	if (intern->std.ce->parent) {
		zend_get_std_object_handlers()->unset_dimension(object, zv_offset);
		return;
	}
	
	zend_throw_exception(NULL, "Cannot unset offsets in a typed array", 0);
}

static int array_buffer_view_compare_objects(zval *obj1, zval *obj2)
{
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

static HashTable *array_buffer_view_get_debug_info(zval *obj, int *is_temp)
{
	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(obj);
	HashTable *props = Z_OBJPROP_P(obj);
	HashTable *ht;
	int i;

	ALLOC_HASHTABLE(ht);
	ZEND_INIT_SYMTABLE_EX(ht, intern->length + zend_hash_num_elements(props), 0);
	zend_hash_copy(ht, props, (copy_ctor_func_t) zval_add_ref);

	*is_temp = 1;

	for (i = 0; i < intern->length; ++i) {
		zval value;
		buffer_view_offset_get(intern, i, &value);
		zend_hash_index_update(ht, i, &value);
	}
	
	return ht;
}

static HashTable *array_buffer_view_get_properties(zval *obj)
{
	buffer_view_object *intern = Z_BUFFER_VIEW_OBJ_P(obj);
	HashTable *ht = zend_std_get_properties(obj);
        zend_string *key;
	zval zv;

	if (Z_ISUNDEF(intern->buffer_zval)) {
		return ht;
	}

        key = zend_string_init("buffer", sizeof("buffer")-1, 0);
        ZVAL_COPY(&zv, &intern->buffer_zval);
	zend_hash_update(ht, key, &zv);
        zend_string_release(key);

	ZVAL_LONG(&zv, intern->offset);
        key = zend_string_init("offset", sizeof("offset")-1, 0);
	zend_hash_update(ht, key, &zv);
        zend_string_release(key);

	ZVAL_LONG(&zv, intern->length);
        key = zend_string_init("length", sizeof("length")-1, 0);
	zend_hash_update(ht, key, &zv);
        zend_string_release(key);

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
	buffer_view_iterator_rewind
};

zend_object_iterator *buffer_view_get_iterator(zend_class_entry *ce, zval *object, int by_ref)
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

PHP_FUNCTION(array_buffer_view_ctor)
{
	zval *buffer_zval;
	long offset = 0, length = 0;
	buffer_view_object *view_intern;
	buffer_object *buffer_intern;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "O|ll", &buffer_zval, array_buffer_ce, &offset, &length) == FAILURE) {
		return;
	}

	view_intern   = Z_BUFFER_VIEW_OBJ_P(getThis());
	buffer_intern = Z_BUFFER_OBJ_P(buffer_zval);

	if (offset < 0) {
		zend_throw_exception(NULL, "Offset must be non-negative", 0);
		return;
	}
	if (offset >= buffer_intern->length) {
		zend_throw_exception(NULL, "Offset has to be smaller than the buffer length", 0);
		return;
	}
	if (length < 0) {
		zend_throw_exception(NULL, "Length must be positive or zero", 0);
		return;
	}

	view_intern->offset = offset;
	ZVAL_COPY(&view_intern->buffer_zval, buffer_zval);

	{
		size_t bytes_per_element = buffer_view_get_bytes_per_element(view_intern);
		size_t max_length = (buffer_intern->length - offset) / bytes_per_element;

		if (length == 0) {
			view_intern->length = max_length;
		} else if (length > max_length) {
			zend_throw_exception(NULL, "Length is larger than the buffer", 0);
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
	zval *buffer_zv, *offset_zv, *length_zv;
        zend_string *key;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	intern = Z_BUFFER_VIEW_OBJ_P(getThis());

	if (!Z_ISUNDEF(intern->buffer_zval)) {
		zend_throw_exception(
			NULL, "Cannot call __wakeup() on an already constructed object", 0
		);
		return;
	}

	props = zend_std_get_properties(getThis());

        key = zend_string_init("buffer", sizeof("buffer")-1, 0);
	buffer_zv = zend_hash_find(props, key);
        zend_string_release(key);

        key = zend_string_init("offset", sizeof("offset")-1, 0);
	offset_zv = zend_hash_find(props, key);
        zend_string_release(key);

        key = zend_string_init("length", sizeof("length")-1, 0);
	length_zv = zend_hash_find(props, key);
        zend_string_release(key);

	if(buffer_zv != NULL
		 && offset_zv != NULL
		 && length_zv != NULL
		 && Z_TYPE_P(buffer_zv) == IS_OBJECT
		 && Z_TYPE_P(offset_zv) == IS_LONG && Z_LVAL_P(offset_zv) >= 0
		 && Z_TYPE_P(length_zv) == IS_LONG && Z_LVAL_P(length_zv) > 0
		 && instanceof_function(Z_OBJCE_P(buffer_zv), array_buffer_ce)
	) {
		buffer_object *buffer_intern = Z_BUFFER_OBJ_P(buffer_zv);
		size_t offset = Z_LVAL_P(offset_zv), length = Z_LVAL_P(length_zv);
		size_t bytes_per_element = buffer_view_get_bytes_per_element(intern);
		size_t max_length = (buffer_intern->length - offset) / bytes_per_element;

		if (offset < buffer_intern->length && length <= max_length) {
			ZVAL_COPY(&intern->buffer_zval, buffer_zv);

			intern->offset = offset;
			intern->length = length;

			intern->buf.as_int8 = buffer_intern->buffer;
			intern->buf.as_int8 += offset;

			return;
		}
	}

	zend_throw_exception(
		NULL, "Invalid serialization data", 0
	);
}

PHP_FUNCTION(array_buffer_view_offset_get)
{
	buffer_view_object *intern;
	long offset;
	zval retval;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &offset) == FAILURE) {
		return;
	}

	intern = Z_BUFFER_VIEW_OBJ_P(getThis());

	if (offset < 0 || offset >= intern->length) {
		zend_throw_exception(NULL, "Offset is outside the buffer range", 0);
		return;
	}

	buffer_view_offset_get(intern, offset, &retval);
	RETURN_ZVAL(&retval, 1, 0);
}

PHP_FUNCTION(array_buffer_view_offset_set)
{
	buffer_view_object *intern;
	long offset;
	zval *value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lz", &offset, &value) == FAILURE) {
		return;
	}

	intern = Z_BUFFER_VIEW_OBJ_P(getThis());

	if (offset < 0 || offset >= intern->length) {
		zend_throw_exception(NULL, "Offset is outside the buffer range", 0);
		return;
	}

	buffer_view_offset_set(intern, offset, value);
}

PHP_FUNCTION(array_buffer_view_offset_exists)
{
	buffer_view_object *intern;
	long offset;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &offset) == FAILURE) {
		return;
	}

	intern = Z_BUFFER_VIEW_OBJ_P(getThis());

	RETURN_BOOL(offset < intern->length);
}

PHP_FUNCTION(array_buffer_view_offset_unset)
{
	long offset;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &offset) == FAILURE) {
		return;
	}

	/* I don't think that the unset() operations makes sense on typed arrays. If you want
	 * to zero out an offset just assign 0 to it. */
	zend_throw_exception(NULL, "Cannot unset offsets in a typed array", 0);
}

PHP_FUNCTION(array_buffer_view_rewind)
{
	buffer_view_object *intern;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	intern = Z_BUFFER_VIEW_OBJ_P(getThis());
	intern->current_offset = 0;
}

PHP_FUNCTION(array_buffer_view_next)
{
	buffer_view_object *intern;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	intern = Z_BUFFER_VIEW_OBJ_P(getThis());
	intern->current_offset++;
}

PHP_FUNCTION(array_buffer_view_valid)
{
	buffer_view_object *intern;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	intern = Z_BUFFER_VIEW_OBJ_P(getThis());
	RETURN_BOOL(intern->current_offset < intern->length);
}

PHP_FUNCTION(array_buffer_view_key)
{
	buffer_view_object *intern;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	intern = Z_BUFFER_VIEW_OBJ_P(getThis());
	RETURN_LONG((long) intern->current_offset);
}

PHP_FUNCTION(array_buffer_view_current)
{
	buffer_view_object *intern;
	zval value;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	intern = Z_BUFFER_VIEW_OBJ_P(getThis());
	buffer_view_offset_get(intern, intern->current_offset, &value);
	RETURN_ZVAL(&value, 1, 0);
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
	PHP_ME(ArrayBuffer, __construct, arginfo_buffer_ctor,        ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(ArrayBuffer, serialize,   arginfo_buffer_void,        ZEND_ACC_PUBLIC)
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
	PHP_ME_MAPPING(__construct, array_buffer_view_ctor,           arginfo_buffer_view_ctor, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME_MAPPING(__wakeup,    array_buffer_view_wakeup,         arginfo_buffer_view_void, ZEND_ACC_PUBLIC)

	/* ArrayAccess */
	PHP_ME_MAPPING(offsetGet,    array_buffer_view_offset_get,    arginfo_buffer_view_offset,     ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(offsetSet,    array_buffer_view_offset_set,    arginfo_buffer_view_offset_set, ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(offsetExists, array_buffer_view_offset_exists, arginfo_buffer_view_offset,     ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(offsetUnset,  array_buffer_view_offset_unset,  arginfo_buffer_view_offset,     ZEND_ACC_PUBLIC)

	/* Iterator */
	PHP_ME_MAPPING(rewind,       array_buffer_view_rewind,        arginfo_buffer_view_void, ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(next,         array_buffer_view_next,          arginfo_buffer_view_void, ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(valid,        array_buffer_view_valid,         arginfo_buffer_view_void, ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(key,          array_buffer_view_key,           arginfo_buffer_view_void, ZEND_ACC_PUBLIC)
	PHP_ME_MAPPING(current,      array_buffer_view_current,       arginfo_buffer_view_void, ZEND_ACC_PUBLIC)

	PHP_FE_END
};

static PHP_MINIT_FUNCTION(buffer)
{
	zend_class_entry tmp_ce;

	INIT_CLASS_ENTRY(tmp_ce, "ArrayBuffer", array_buffer_functions);
	array_buffer_ce = zend_register_internal_class(&tmp_ce);
	array_buffer_ce->create_object = array_buffer_create_object;
	memcpy(&array_buffer_handlers, zend_get_std_object_handlers(), sizeof(array_buffer_handlers));
	array_buffer_handlers.offset = XtOffsetOf(buffer_object, std);
        array_buffer_handlers.dtor_obj  = zend_objects_destroy_object;
	array_buffer_handlers.free_obj  = array_buffer_free;
	array_buffer_handlers.clone_obj = array_buffer_clone;

	zend_class_implements(array_buffer_ce, 1, zend_ce_serializable);

#define DEFINE_ARRAY_BUFFER_VIEW_CLASS(class_name, type)                         \
	INIT_CLASS_ENTRY(tmp_ce, #class_name, array_buffer_view_functions);      \
	type##_array_ce = zend_register_internal_class(&tmp_ce);                 \
	type##_array_ce->create_object = array_buffer_view_create_object;        \
	type##_array_ce->get_iterator = buffer_view_get_iterator;                \
	zend_class_implements(type##_array_ce, 2,                                \
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

	memcpy(&array_buffer_view_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	array_buffer_view_handlers.offset = XtOffsetOf(buffer_view_object, std);
        array_buffer_view_handlers.dtor_obj        = zend_objects_destroy_object;
	array_buffer_view_handlers.clone_obj       = array_buffer_view_clone;
	array_buffer_view_handlers.read_dimension  = array_buffer_view_read_dimension;
	array_buffer_view_handlers.write_dimension = array_buffer_view_write_dimension;
	array_buffer_view_handlers.has_dimension   = array_buffer_view_has_dimension;
	array_buffer_view_handlers.unset_dimension = array_buffer_view_unset_dimension;
	array_buffer_view_handlers.compare_objects = array_buffer_view_compare_objects;
	array_buffer_view_handlers.get_debug_info  = array_buffer_view_get_debug_info;
	array_buffer_view_handlers.free_obj        = array_buffer_view_free;
	array_buffer_view_handlers.get_properties  = array_buffer_view_get_properties;

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
