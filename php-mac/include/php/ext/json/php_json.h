/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Omar Kilani <omar@php.net>                                   |
  +----------------------------------------------------------------------+
*/

/* $Id: php_json.h 293036 2010-01-03 09:23:27Z sebastian $ */

#ifndef PHP_JSON_H
#define PHP_JSON_H

#define PHP_JSON_VERSION "1.2.1"

#include "ext/standard/php_smart_str.h"

extern zend_module_entry json_module_entry;
#define phpext_json_ptr &json_module_entry

#ifdef PHP_WIN32
#define PHP_JSON_API __declspec(dllexport)
#else
#define PHP_JSON_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef ZTS
#define JSON_G(v) TSRMG(json_globals_id, zend_json_globals *, v)
#else
#define JSON_G(v) (json_globals.v)
#endif

PHP_JSON_API void php_json_encode(smart_str *buf, zval *val TSRMLS_DC);
PHP_JSON_API void php_json_decode(zval *return_value, char *buf, int buf_len, zend_bool assoc TSRMLS_DC);

#endif  /* PHP_JSON_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
