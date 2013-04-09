/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
*/
namespace HPHP {

/*
bool HPHP::f_is_bool(HPHP::Variant const&)
_ZN4HPHP9f_is_boolERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_bool(TypedValue* var) asm("_ZN4HPHP9f_is_boolERKNS_7VariantE");

/*
bool HPHP::f_is_int(HPHP::Variant const&)
_ZN4HPHP8f_is_intERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_int(TypedValue* var) asm("_ZN4HPHP8f_is_intERKNS_7VariantE");

/*
bool HPHP::f_is_integer(HPHP::Variant const&)
_ZN4HPHP12f_is_integerERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_integer(TypedValue* var) asm("_ZN4HPHP12f_is_integerERKNS_7VariantE");

/*
bool HPHP::f_is_long(HPHP::Variant const&)
_ZN4HPHP9f_is_longERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_long(TypedValue* var) asm("_ZN4HPHP9f_is_longERKNS_7VariantE");

/*
bool HPHP::f_is_double(HPHP::Variant const&)
_ZN4HPHP11f_is_doubleERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_double(TypedValue* var) asm("_ZN4HPHP11f_is_doubleERKNS_7VariantE");

/*
bool HPHP::f_is_float(HPHP::Variant const&)
_ZN4HPHP10f_is_floatERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_float(TypedValue* var) asm("_ZN4HPHP10f_is_floatERKNS_7VariantE");

/*
bool HPHP::f_is_numeric(HPHP::Variant const&)
_ZN4HPHP12f_is_numericERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_numeric(TypedValue* var) asm("_ZN4HPHP12f_is_numericERKNS_7VariantE");

/*
bool HPHP::f_is_real(HPHP::Variant const&)
_ZN4HPHP9f_is_realERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_real(TypedValue* var) asm("_ZN4HPHP9f_is_realERKNS_7VariantE");

/*
bool HPHP::f_is_string(HPHP::Variant const&)
_ZN4HPHP11f_is_stringERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_string(TypedValue* var) asm("_ZN4HPHP11f_is_stringERKNS_7VariantE");

/*
bool HPHP::f_is_scalar(HPHP::Variant const&)
_ZN4HPHP11f_is_scalarERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_scalar(TypedValue* var) asm("_ZN4HPHP11f_is_scalarERKNS_7VariantE");

/*
bool HPHP::f_is_array(HPHP::Variant const&)
_ZN4HPHP10f_is_arrayERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_array(TypedValue* var) asm("_ZN4HPHP10f_is_arrayERKNS_7VariantE");

/*
bool HPHP::f_is_object(HPHP::Variant const&)
_ZN4HPHP11f_is_objectERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_object(TypedValue* var) asm("_ZN4HPHP11f_is_objectERKNS_7VariantE");

/*
bool HPHP::f_is_resource(HPHP::Variant const&)
_ZN4HPHP13f_is_resourceERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_resource(TypedValue* var) asm("_ZN4HPHP13f_is_resourceERKNS_7VariantE");

/*
bool HPHP::f_is_null(HPHP::Variant const&)
_ZN4HPHP9f_is_nullERKNS_7VariantE

(return value) => rax
var => rdi
*/

bool fh_is_null(TypedValue* var) asm("_ZN4HPHP9f_is_nullERKNS_7VariantE");

/*
HPHP::String HPHP::f_gettype(HPHP::Variant const&)
_ZN4HPHP9f_gettypeERKNS_7VariantE

(return value) => rax
_rv => rdi
v => rsi
*/

Value* fh_gettype(Value* _rv, TypedValue* v) asm("_ZN4HPHP9f_gettypeERKNS_7VariantE");

/*
HPHP::String HPHP::f_get_resource_type(HPHP::Object const&)
_ZN4HPHP19f_get_resource_typeERKNS_6ObjectE

(return value) => rax
_rv => rdi
handle => rsi
*/

Value* fh_get_resource_type(Value* _rv, Value* handle) asm("_ZN4HPHP19f_get_resource_typeERKNS_6ObjectE");

/*
long HPHP::f_intval(HPHP::Variant const&, long)
_ZN4HPHP8f_intvalERKNS_7VariantEl

(return value) => rax
v => rdi
base => rsi
*/

long fh_intval(TypedValue* v, long base) asm("_ZN4HPHP8f_intvalERKNS_7VariantEl");

/*
double HPHP::f_doubleval(HPHP::Variant const&)
_ZN4HPHP11f_doublevalERKNS_7VariantE

(return value) => xmm0
v => rdi
*/

double fh_doubleval(TypedValue* v) asm("_ZN4HPHP11f_doublevalERKNS_7VariantE");

/*
double HPHP::f_floatval(HPHP::Variant const&)
_ZN4HPHP10f_floatvalERKNS_7VariantE

(return value) => xmm0
v => rdi
*/

double fh_floatval(TypedValue* v) asm("_ZN4HPHP10f_floatvalERKNS_7VariantE");

/*
HPHP::String HPHP::f_strval(HPHP::Variant const&)
_ZN4HPHP8f_strvalERKNS_7VariantE

(return value) => rax
_rv => rdi
v => rsi
*/

Value* fh_strval(Value* _rv, TypedValue* v) asm("_ZN4HPHP8f_strvalERKNS_7VariantE");

/*
bool HPHP::f_settype(HPHP::VRefParamValue const&, HPHP::String const&)
_ZN4HPHP9f_settypeERKNS_14VRefParamValueERKNS_6StringE

(return value) => rax
var => rdi
type => rsi
*/

bool fh_settype(TypedValue* var, Value* type) asm("_ZN4HPHP9f_settypeERKNS_14VRefParamValueERKNS_6StringE");

/*
HPHP::Variant HPHP::f_print_r(HPHP::Variant const&, bool)
_ZN4HPHP9f_print_rERKNS_7VariantEb

(return value) => rax
_rv => rdi
expression => rsi
ret => rdx
*/

TypedValue* fh_print_r(TypedValue* _rv, TypedValue* expression, bool ret) asm("_ZN4HPHP9f_print_rERKNS_7VariantEb");

/*
HPHP::Variant HPHP::f_var_export(HPHP::Variant const&, bool)
_ZN4HPHP12f_var_exportERKNS_7VariantEb

(return value) => rax
_rv => rdi
expression => rsi
ret => rdx
*/

TypedValue* fh_var_export(TypedValue* _rv, TypedValue* expression, bool ret) asm("_ZN4HPHP12f_var_exportERKNS_7VariantEb");

/*
void HPHP::f_var_dump(int, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP10f_var_dumpEiRKNS_7VariantERKNS_5ArrayE

_argc => rdi
expression => rsi
_argv => rdx
*/

void fh_var_dump(int64_t _argc, TypedValue* expression, Value* _argv) asm("_ZN4HPHP10f_var_dumpEiRKNS_7VariantERKNS_5ArrayE");

/*
void HPHP::f_debug_zval_dump(HPHP::Variant const&)
_ZN4HPHP17f_debug_zval_dumpERKNS_7VariantE

variable => rdi
*/

void fh_debug_zval_dump(TypedValue* variable) asm("_ZN4HPHP17f_debug_zval_dumpERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_unserialize(HPHP::String const&)
_ZN4HPHP13f_unserializeERKNS_6StringE

(return value) => rax
_rv => rdi
str => rsi
*/

TypedValue* fh_unserialize(TypedValue* _rv, Value* str) asm("_ZN4HPHP13f_unserializeERKNS_6StringE");

/*
HPHP::Array HPHP::f_get_defined_vars()
_ZN4HPHP18f_get_defined_varsEv

(return value) => rax
_rv => rdi
*/

Value* fh_get_defined_vars(Value* _rv) asm("_ZN4HPHP18f_get_defined_varsEv");

/*
bool HPHP::f_import_request_variables(HPHP::String const&, HPHP::String const&)
_ZN4HPHP26f_import_request_variablesERKNS_6StringES2_

(return value) => rax
types => rdi
prefix => rsi
*/

bool fh_import_request_variables(Value* types, Value* prefix) asm("_ZN4HPHP26f_import_request_variablesERKNS_6StringES2_");

/*
long HPHP::f_extract(HPHP::Array const&, int, HPHP::String const&)
_ZN4HPHP9f_extractERKNS_5ArrayEiRKNS_6StringE

(return value) => rax
var_array => rdi
extract_type => rsi
prefix => rdx
*/

long fh_extract(Value* var_array, int extract_type, Value* prefix) asm("_ZN4HPHP9f_extractERKNS_5ArrayEiRKNS_6StringE");


} // !HPHP

