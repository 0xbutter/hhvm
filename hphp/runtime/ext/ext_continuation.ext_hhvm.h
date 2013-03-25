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
HPHP::Object HPHP::f_hphp_create_continuation(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP26f_hphp_create_continuationERKNS_6StringES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
clsname => rsi
funcname => rdx
origFuncName => rcx
args => r8
*/

Value* fh_hphp_create_continuation(Value* _rv, Value* clsname, Value* funcname, Value* origFuncName, Value* args) asm("_ZN4HPHP26f_hphp_create_continuationERKNS_6StringES2_S2_RKNS_5ArrayE");


} // !HPHP

