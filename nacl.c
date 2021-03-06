/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Gasol Wu <gasol.wu@gmai.com>                                 |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_nacl.h"
#include "crypto_auth.h"
#include "crypto_box.h"
#include "crypto_hash.h"
#include "crypto_secretbox.h"
#include "crypto_stream.h"
#include "crypto_sign.h"
#include "crypto_onetimeauth.h"

/* If you declare any globals in php_nacl.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(nacl)
*/

/* True global resources - no need for thread safety here */
static int le_nacl;

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_auth, 0, 0, 2)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_auth_verify, 0, 0, 3)
	ZEND_ARG_INFO(0, crypted)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_box, 0, 0, 4)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, nonce)
	ZEND_ARG_INFO(0, public_key)
	ZEND_ARG_INFO(0, secret_key)
	ZEND_ARG_INFO(0, raw_output)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_box_open, 0, 0, 4)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, nonce)
	ZEND_ARG_INFO(0, public_key)
	ZEND_ARG_INFO(0, secret_key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_box_keypair, 0, 0, 2)
	ZEND_ARG_INFO(1, public_key)
	ZEND_ARG_INFO(1, secret_key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_stream, 0, 0, 3)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, nonce)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_stream_xor, 0, 0, 3)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, nonce)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_secretbox, 0, 0, 3)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, nonce)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_secretbox_open, 0, 0, 3)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, nonce)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_sign, 0, 0, 2)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, secret_key)
	ZEND_ARG_INFO(0, raw_output)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_sign_open, 0, 0, 2)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, public_key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_sign_keypair, 0, 0, 2)
	ZEND_ARG_INFO(1, public_key)
	ZEND_ARG_INFO(1, secret_key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_onetimeauth, 0, 0, 2)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_onetimeauth_verify, 0, 0, 3)
	ZEND_ARG_INFO(0, crypted)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nacl_crypto_hash, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ nacl_functions[]
 *
 * Every user visible function must have an entry in nacl_functions[].
 */
const zend_function_entry nacl_functions[] = {
	PHP_FE(nacl_crypto_auth, arginfo_nacl_crypto_auth)
	PHP_FE(nacl_crypto_auth_verify, arginfo_nacl_crypto_auth_verify)
	PHP_FE(nacl_crypto_box, arginfo_nacl_crypto_box)
	PHP_FE(nacl_crypto_box_open, arginfo_nacl_crypto_box_open)
	PHP_FE(nacl_crypto_box_keypair, arginfo_nacl_crypto_box_keypair)
	PHP_FE(nacl_crypto_stream, arginfo_nacl_crypto_stream)
	PHP_FE(nacl_crypto_stream_xor, arginfo_nacl_crypto_stream_xor)
	PHP_FE(nacl_crypto_secretbox, arginfo_nacl_crypto_secretbox)
	PHP_FE(nacl_crypto_secretbox_open, arginfo_nacl_crypto_secretbox_open)
	PHP_FE(nacl_crypto_sign, arginfo_nacl_crypto_sign)
	PHP_FE(nacl_crypto_sign_open, arginfo_nacl_crypto_sign_open)
	PHP_FE(nacl_crypto_sign_keypair, arginfo_nacl_crypto_sign_keypair)
	PHP_FE(nacl_crypto_onetimeauth, arginfo_nacl_crypto_onetimeauth)
	PHP_FE(nacl_crypto_onetimeauth_verify, arginfo_nacl_crypto_onetimeauth_verify)
	PHP_FE(nacl_crypto_hash, arginfo_nacl_crypto_hash)
	PHP_FE_END
};
/* }}} */

/* {{{ nacl_module_entry
 */
zend_module_entry nacl_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"nacl",
	nacl_functions,
	PHP_MINIT(nacl),
	PHP_MSHUTDOWN(nacl),
	NULL,
	NULL,
	PHP_MINFO(nacl),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_NACL_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_NACL
ZEND_GET_MODULE(nacl)
#endif

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(nacl)
{
	REGISTER_LONG_CONSTANT("NACL_CRYPTO_AUTH_BYTES", crypto_auth_BYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NACL_CRYPTO_AUTH_KEYBYTES", crypto_auth_KEYBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_AUTH_PRIMITIVE", crypto_auth_PRIMITIVE, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_AUTH_IMPLEMENTATION", crypto_auth_IMPLEMENTATION, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_AUTH_VERSION", crypto_auth_VERSION, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("NACL_CRYPTO_STREAM_KEYBYTES", crypto_stream_KEYBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NACL_CRYPTO_STREAM_NONCEBYTES", crypto_stream_NONCEBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_STREAM_PRIMITIVE", crypto_stream_PRIMITIVE, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_STREAM_IMPLEMENTATION", crypto_stream_IMPLEMENTATION, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_STREAM_VERSION", crypto_stream_VERSION, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("NACL_CRYPTO_SECRETBOX_KEYBYTES", crypto_secretbox_KEYBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NACL_CRYPTO_SECRETBOX_NONCEBYTES", crypto_secretbox_NONCEBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NACL_CRYPTO_SECRETBOX_ZEROBYTES", crypto_secretbox_ZEROBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NACL_CRYPTO_SECRETBOX_BOXZEROBYTES", crypto_secretbox_BOXZEROBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_SECRETBOX_PRIMITIVE", crypto_secretbox_PRIMITIVE, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_SECRETBOX_IMPLEMENTATION", crypto_secretbox_IMPLEMENTATION, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_SECRETBOX_VERSION", crypto_secretbox_VERSION, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("NACL_CRYPTO_ONETIMEAUTH_BYTES", crypto_onetimeauth_BYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NACL_CRYPTO_ONETIMEAUTH_KEYBYTES", crypto_onetimeauth_KEYBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_ONETIMEAUTH_PRIMITIVE", crypto_onetimeauth_PRIMITIVE, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_ONETIMEAUTH_IMPLEMENTATION", crypto_onetimeauth_IMPLEMENTATION, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_ONETIMEAUTH_VERSION", crypto_onetimeauth_VERSION, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("NACL_CRYPTO_HASH_BYTES", crypto_hash_BYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_HASH_PRIMITIVE", crypto_hash_PRIMITIVE, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_HASH_IMPLEMENTATION", crypto_hash_IMPLEMENTATION, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_HASH_VERSION", crypto_hash_VERSION, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("NACL_CRYPTO_SIGN_BYTES", crypto_sign_BYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NACL_CRYPTO_SIGN_PUBLICKEYBYTES", crypto_sign_PUBLICKEYBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NACL_CRYPTO_SIGN_SECRETKEYBYTES", crypto_sign_SECRETKEYBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_SIGN_PRIMITIVE", crypto_sign_PRIMITIVE, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_SIGN_IMPLEMENTATION", crypto_sign_IMPLEMENTATION, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_SIGN_VERSION", crypto_sign_VERSION, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("NACL_CRYPTO_BOX_PUBLICKEYBYTES", crypto_box_PUBLICKEYBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NACL_CRYPTO_BOX_SECRETKEYBYTES", crypto_box_SECRETKEYBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NACL_CRYPTO_BOX_BEFORENMBYTES", crypto_box_BEFORENMBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NACL_CRYPTO_BOX_NONCEBYTES", crypto_box_NONCEBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NACL_CRYPTO_BOX_ZEROBYTES", crypto_box_ZEROBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("NACL_CRYPTO_BOX_BOXZEROBYTES", crypto_box_BOXZEROBYTES, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_BOX_PRIMITIVE", crypto_box_PRIMITIVE, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_BOX_IMPLEMENTATION", crypto_box_IMPLEMENTATION, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("NACL_CRYPTO_BOX_VERSION", crypto_box_VERSION, CONST_CS | CONST_PERSISTENT);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(nacl)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(nacl)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "NaCl support", "enabled");
	php_info_print_table_row(2, "Version", PHP_NACL_VERSION);
	php_info_print_table_end();
}
/* }}} */

/* {{{ nacl_crypto_auth
 */
PHP_FUNCTION(nacl_crypto_auth)
{
	const unsigned char k[crypto_auth_KEYBYTES];
	unsigned char *returnval, *data, *key = NULL;
	int data_len, key_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &data, &data_len, &key, &key_len) == FAILURE) {
		return;
	}

	strncpy((char *) &k, (const char *) key, crypto_auth_KEYBYTES);

	returnval = safe_emalloc(crypto_auth_BYTES, 1, 1);

	if (crypto_auth(returnval, data, data_len, k)) {
		RETURN_FALSE;
	}

	RETURN_STRINGL((char *) returnval, crypto_auth_BYTES, 0);
}
/* }}} */

/* {{{ nacl_crypto_auth_verify
 */
PHP_FUNCTION(nacl_crypto_auth_verify)
{
	unsigned char *hash, *data, *key = NULL;
	int hash_len, data_len, key_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &hash, &hash_len, &data, &data_len, &key, &key_len) == FAILURE) {
		return;
	}

	int ret = crypto_auth_verify(hash, data, data_len, key);
	if (ret) {
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ nacl_crypto_stream
 */
PHP_FUNCTION(nacl_crypto_stream)
{
	const unsigned char k[crypto_stream_KEYBYTES], n[crypto_stream_NONCEBYTES];
	unsigned char *returnval, *data, *key, *nonce = NULL;
	int data_len, key_len, nonce_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &data, &data_len, &nonce, &nonce_len, &key, &key_len) == FAILURE) {
		return;
	}

	strncpy((char *) &k, (const char *) key, crypto_stream_KEYBYTES);
	strncpy((char *) &n, (const char *) nonce, crypto_stream_NONCEBYTES);

	returnval = safe_emalloc(data_len, 1, 1);

	if (crypto_stream(returnval, data_len, n, k)) {
		RETURN_FALSE;
	}

	RETURN_STRINGL((char *) returnval, data_len, 0);
}
/* }}} */

/* {{{ nacl_crypto_stream_xor
 */
PHP_FUNCTION(nacl_crypto_stream_xor)
{
	const unsigned char k[crypto_stream_KEYBYTES], n[crypto_stream_NONCEBYTES];
	unsigned char *returnval, *data, *key, *nonce = NULL;
	int data_len, key_len, nonce_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &data, &data_len, &nonce, &nonce_len, &key, &key_len) == FAILURE) {
		return;
	}

	strncpy((char *) &k, (const char *) key, crypto_stream_KEYBYTES);
	strncpy((char *) &n, (const char *) nonce, crypto_stream_NONCEBYTES);

	returnval = safe_emalloc(data_len, 1, 1);

	if (crypto_stream_xor(returnval, data, data_len, n, k)) {
		RETURN_FALSE;
	}

	RETURN_STRINGL((char *) returnval, data_len, 0);
}
/* }}} */

/* {{{ nacl_crypto_secretbox
 */
PHP_FUNCTION(nacl_crypto_secretbox)
{
	const unsigned char k[crypto_secretbox_KEYBYTES], n[crypto_secretbox_NONCEBYTES];
	unsigned char *returnval, *data, *key, *nonce = NULL;
	int data_len, key_len, nonce_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &data, &data_len, &nonce, &nonce_len, &key, &key_len) == FAILURE) {
		return;
	}

	strncpy((char *) &k, (const char *) key, crypto_secretbox_KEYBYTES);
	strncpy((char *) &n, (const char *) nonce, crypto_secretbox_NONCEBYTES);

	returnval = safe_emalloc(data_len, 1, 1);

	if (crypto_secretbox(returnval, data, data_len, n, k)) {
		RETURN_FALSE;
	}

	RETURN_STRINGL((char *) returnval, data_len, 0);
}
/* }}} */

/* {{{ nacl_crypto_secretbox_open
 */
PHP_FUNCTION(nacl_crypto_secretbox_open)
{
	const unsigned char k[crypto_secretbox_KEYBYTES], n[crypto_secretbox_NONCEBYTES];
	unsigned char *returnval, *data, *key, *nonce = NULL;
	int data_len, key_len, nonce_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &data, &data_len, &nonce, &nonce_len, &key, &key_len) == FAILURE) {
		return;
	}

	strncpy((char *) &k, (const char *) key, crypto_secretbox_KEYBYTES);
	strncpy((char *) &n, (const char *) nonce, crypto_secretbox_NONCEBYTES);

	returnval = safe_emalloc(data_len, 1, 1);

	if (crypto_secretbox_open(returnval, data, data_len, n, k)) {
		RETURN_FALSE;
	}

	RETURN_STRINGL((char *) returnval, data_len, 0);
}
/* }}} */

/* {{{ nacl_crypto_onetimeauth
 */
PHP_FUNCTION(nacl_crypto_onetimeauth)
{
	const unsigned char k[crypto_onetimeauth_KEYBYTES];
	unsigned char *returnval, *data, *key = NULL;
	int data_len, key_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &data, &data_len, &key, &key_len) == FAILURE) {
		return;
	}

	strncpy((char *) &k, (const char *) key, crypto_onetimeauth_KEYBYTES);

	returnval = safe_emalloc(crypto_onetimeauth_BYTES, 1, 1);

	if (crypto_onetimeauth(returnval, data, data_len, k)) {
		RETURN_FALSE;
	}

	RETURN_STRINGL((char *) returnval, crypto_onetimeauth_BYTES, 0);
}
/* }}} */

/* {{{ nacl_crypto_onetimeauth_verify
 */
PHP_FUNCTION(nacl_crypto_onetimeauth_verify)
{
	const unsigned char k[crypto_onetimeauth_KEYBYTES];
	unsigned char *auth, *data, *key = NULL;
	int auth_len, data_len, key_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &auth, &auth_len, &data, &data_len, &key, &key_len) == FAILURE) {
		return;
	}

	strncpy((char *) &k, (const char *) key, crypto_onetimeauth_KEYBYTES);

	if (crypto_onetimeauth_verify(auth, data, data_len, k)) {
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ nacl_crypto_hash
 */
PHP_FUNCTION(nacl_crypto_hash)
{
	unsigned char *returnval, *data = NULL;
	int data_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &data_len) == FAILURE) {
		return;
	}

	returnval = safe_emalloc(crypto_hash_BYTES, 1, 1);

	if (crypto_hash(returnval, data, data_len)) {
		RETURN_FALSE;
	}

	RETURN_STRINGL((char *) returnval, crypto_hash_BYTES, 0);
}
/* }}} */

/* {{{ nacl_crypto_sign_keypair
 */
PHP_FUNCTION(nacl_crypto_sign_keypair)
{
	unsigned char pk[crypto_sign_PUBLICKEYBYTES], sk[crypto_sign_SECRETKEYBYTES];
	zval *pubkey = NULL, *seckey = NULL;
	zend_bool raw_output = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|b", &pubkey, &seckey, &raw_output) == FAILURE) {
		return;
	}

	if (pubkey) {
		zval_dtor(pubkey);
	}
	if (seckey) {
		zval_dtor(seckey);
	}

	if (crypto_sign_keypair(pk, sk)) {
		RETURN_FALSE;
	}

	if (raw_output) {
		ZVAL_STRINGL(pubkey, ((const char *) &pk), crypto_sign_PUBLICKEYBYTES, 1);
		ZVAL_STRINGL(seckey, ((const char *) &sk), crypto_sign_SECRETKEYBYTES, 1);
	} else {
		char *pubkey_digest = safe_emalloc(sizeof(char), crypto_sign_PUBLICKEYBYTES * 2, 0);
		char *seckey_digest = safe_emalloc(sizeof(char), crypto_sign_SECRETKEYBYTES * 2, 0);
		php_nacl_bin2hex(pubkey_digest, ((const unsigned char *) &pk), crypto_sign_PUBLICKEYBYTES);
		php_nacl_bin2hex(seckey_digest, ((const unsigned char *) &sk), crypto_sign_SECRETKEYBYTES);
		ZVAL_STRINGL(pubkey, pubkey_digest, crypto_sign_PUBLICKEYBYTES * 2, 0);
		ZVAL_STRINGL(seckey, seckey_digest, crypto_sign_SECRETKEYBYTES * 2, 0);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ nacl_crypto_sign
 */
PHP_FUNCTION(nacl_crypto_sign)
{
	unsigned char sk[crypto_sign_SECRETKEYBYTES];
	unsigned char *sm = NULL, *data = NULL, *secretkey = NULL;
	int data_len = 0, secretkey_len = 0, sm_len_max;
	unsigned long long sm_len = 0;
	zend_bool raw_output = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|b", &data, &data_len, &secretkey, &secretkey_len, &raw_output) == FAILURE) {
		return;
	}

	strncpy((char *) &sk, (const char *) secretkey, crypto_sign_SECRETKEYBYTES);

	sm_len_max = data_len + crypto_sign_BYTES;
	sm = safe_emalloc(sizeof(char), sm_len_max, 0);

	if (crypto_sign(sm, &sm_len, data, data_len, sk)) {
		RETURN_FALSE;
	}

	if (raw_output) {
		RETURN_STRINGL((const char *) sm, sm_len, 0);
	} else {
		int sm_digest_len = sm_len_max * 2;
		char *sm_digest = safe_emalloc(sizeof(char), sm_digest_len, 0);
		php_nacl_bin2hex(sm_digest, ((const unsigned char *) sm), sm_len);
		efree(sm);
		RETURN_STRINGL(sm_digest, sm_digest_len, 0);
	}
}
/* }}} */

/* {{{ nacl_crypto_sign_open
 */
PHP_FUNCTION(nacl_crypto_sign_open)
{
	unsigned char pk[crypto_sign_PUBLICKEYBYTES];
	unsigned char *returnvalue = NULL, *data = NULL, *pubkey = NULL;
	int data_len = 0, pubkey_len = 0;
	unsigned long long returnvalue_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &data, &data_len, &pubkey, &pubkey_len) == FAILURE) {
		return;
	}

	strncpy((char *) &pk, (const char *) pubkey, crypto_sign_PUBLICKEYBYTES);

	returnvalue = safe_emalloc(sizeof(char), data_len, 0);

	if (crypto_sign_open(returnvalue, &returnvalue_len, data, data_len, pk)) {
		RETURN_FALSE;
	}

	RETURN_STRINGL((const char *) returnvalue, returnvalue_len, 0);
}
/* }}} */

/* {{{ nacl_crypto_box_keypair
 */
PHP_FUNCTION(nacl_crypto_box_keypair)
{
	unsigned char pk[crypto_box_PUBLICKEYBYTES], sk[crypto_box_SECRETKEYBYTES];
	zval *pubkey = NULL, *seckey = NULL;
	zend_bool raw_output = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|b", &pubkey, &seckey, &raw_output) == FAILURE) {
		return;
	}

	if (pubkey) {
		zval_dtor(pubkey);
	}
	if (seckey) {
		zval_dtor(seckey);
	}

	if (crypto_box_keypair(pk, sk)) {
		RETURN_FALSE;
	}

	if (raw_output) {
		ZVAL_STRINGL(pubkey, ((const char *) &pk), crypto_box_PUBLICKEYBYTES, 1);
		ZVAL_STRINGL(seckey, ((const char *) &sk), crypto_box_SECRETKEYBYTES, 1);
	} else {
		char *pubkey_digest = safe_emalloc(sizeof(char), crypto_box_PUBLICKEYBYTES * 2, 0);
		char *seckey_digest = safe_emalloc(sizeof(char), crypto_box_SECRETKEYBYTES * 2, 0);
		php_nacl_bin2hex(pubkey_digest, ((const unsigned char *) &pk), crypto_box_PUBLICKEYBYTES);
		php_nacl_bin2hex(seckey_digest, ((const unsigned char *) &sk), crypto_box_SECRETKEYBYTES);
		ZVAL_STRINGL(pubkey, pubkey_digest, crypto_box_PUBLICKEYBYTES * 2, 0);
		ZVAL_STRINGL(seckey, seckey_digest, crypto_box_SECRETKEYBYTES * 2, 0);
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ nacl_crypto_box
 */
PHP_FUNCTION(nacl_crypto_box)
{
	unsigned char pk[crypto_box_PUBLICKEYBYTES], sk[crypto_box_SECRETKEYBYTES], n[crypto_box_NONCEBYTES];
	unsigned char *returnvalue = NULL, *m = NULL, *data = NULL, *nonce = NULL, *pubkey = NULL, *seckey = NULL;
	int m_len = 0, data_len = 0, nonce_len = 0, pubkey_len = 0, seckey_len = 0;
	unsigned long long sm_len = 0;
	zend_bool raw_output = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssss|b", &data, &data_len,
				&nonce, &nonce_len, &pubkey, &pubkey_len, &seckey, &seckey_len, &raw_output) == FAILURE) {
		return;
	}

	strncpy((char *) &sk, (const char *) seckey, crypto_box_SECRETKEYBYTES);
	strncpy((char *) &pk, (const char *) pubkey, crypto_box_PUBLICKEYBYTES);
	strncpy((char *) &n, (const char *) nonce, crypto_box_NONCEBYTES);
	m_len = data_len + crypto_box_ZEROBYTES;
	returnvalue = safe_emalloc(sizeof(unsigned char), m_len, 0);
	m = safe_emalloc(sizeof(char), m_len, 0);
	memset(m, 0, crypto_box_ZEROBYTES);
	strncpy((char *) m + crypto_box_ZEROBYTES, (const char *) data, data_len);

	if (crypto_box(returnvalue, m, m_len, n, pk, sk)) {
		RETURN_FALSE;
	}

	if (raw_output) {
		RETURN_STRINGL((const char *) returnvalue + crypto_box_BOXZEROBYTES, (m_len - crypto_box_BOXZEROBYTES), 1);
	} else {
		int digest_len = m_len * 2;
		char *digest = safe_emalloc(sizeof(char), digest_len, 0);
		php_nacl_bin2hex(digest, (const unsigned char *) returnvalue + crypto_box_BOXZEROBYTES,
				m_len - crypto_box_BOXZEROBYTES);
		efree(returnvalue);
		RETURN_STRINGL(digest, (m_len - crypto_box_BOXZEROBYTES) * 2, 0);
	}
}
/* }}} */

/* {{{ nacl_crypto_box_open
 */
PHP_FUNCTION(nacl_crypto_box_open)
{
	unsigned char pk[crypto_box_PUBLICKEYBYTES], sk[crypto_box_SECRETKEYBYTES], n[crypto_box_NONCEBYTES];
	unsigned char *returnvalue = NULL, *c = NULL, *data = NULL, *nonce = NULL, *pubkey = NULL, *seckey = NULL;
	int c_len = 0, data_len = 0, nonce_len = 0, pubkey_len = 0, seckey_len = 0;
	unsigned long long sm_len = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssss", &data, &data_len,
				&nonce, &nonce_len, &pubkey, &pubkey_len, &seckey, &seckey_len) == FAILURE) {
		return;
	}

	strncpy((char *) &sk, (const char *) seckey, crypto_box_SECRETKEYBYTES);
	strncpy((char *) &pk, (const char *) pubkey, crypto_box_PUBLICKEYBYTES);
	strncpy((char *) &n, (const char *) nonce, crypto_box_NONCEBYTES);
	c_len = data_len + crypto_box_BOXZEROBYTES;
	returnvalue = safe_emalloc(sizeof(char), c_len, 0);
	c = safe_emalloc(sizeof(char), c_len, 0);
	memset(c, 0, crypto_box_BOXZEROBYTES);
	strncpy((char *) c + crypto_box_BOXZEROBYTES, (const char *) data, data_len);

	if (crypto_box_open(returnvalue, c, c_len, n, pk, sk)) {
		RETURN_FALSE;
	}

	RETURN_STRINGL((const char *) returnvalue + crypto_box_ZEROBYTES, c_len - crypto_box_ZEROBYTES, 0);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
