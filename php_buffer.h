#ifndef PHP_BUFFER_H
#define PHP_BUFFER_H

#define BUFFER_VERSION "0.2"

extern zend_module_entry buffer_module_entry;
#define phpext_buffer_ptr &buffer_module_entry

#ifdef PHP_WIN32
#      define PHP_BUFFER_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#      define PHP_BUFFER_API __attribute__ ((visibility("default")))
#else
#      define PHP_BUFFER_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif
#pragma once

#if defined(ZTS) && defined(COMPILE_DL_BUFFER)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

typedef struct _buffer_object {
  void *buffer;
  size_t length;

  zend_object std;
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
	zval buffer_zval;

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

	zend_object std;
} buffer_view_object;

typedef struct _buffer_view_iterator {
	zend_object_iterator intern;
	buffer_view_object *view;
	size_t offset;
	zval current;
} buffer_view_iterator;

static inline buffer_object * php_buffer_fetch_object(zend_object *obj) {
      return (buffer_object *)((char *)obj - XtOffsetOf(buffer_object, std));
}
#define Z_BUFFER_OBJ_P(zvp) php_buffer_fetch_object(Z_OBJ_P(zvp));

static inline buffer_view_object * php_buffer_view_fetch_object(zend_object *obj) {
      return (buffer_view_object *)((char *)obj - XtOffsetOf(buffer_view_object, std));
}
#define Z_BUFFER_VIEW_OBJ_P(zvp) php_buffer_view_fetch_object(Z_OBJ_P(zvp));

static PHP_MINIT_FUNCTION(buffer);
static PHP_MSHUTDOWN_FUNCTION(buffer);

#endif


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
