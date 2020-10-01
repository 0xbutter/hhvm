/*
  +----------------------------------------------------------------------+
  | HipHop for PHP                                                       |
  +----------------------------------------------------------------------+
  | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef HPHP_LOGGING_ARRAY_H_
#define HPHP_LOGGING_ARRAY_H_

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/bespoke/entry-types.h"
#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP { namespace bespoke {

struct LoggingProfile;

struct LoggingArray : BespokeArray {
  static LoggingArray* asLogging(ArrayData* ad);
  static const LoggingArray* asLogging(const ArrayData* ad);
  static LoggingArray* Make(ArrayData* ad, LoggingProfile* profile,
                            EntryTypes ms);
  static LoggingArray* MakeStatic(ArrayData* ad, LoggingProfile* profile);
  static void FreeStatic(LoggingArray* lad);
  static const Layout* layout();

  // Update m_kind and m_size after doing a mutation on the wrapped array.
  void updateKindAndSize();

  // Record that this array reached a given profiling tracelet.
  void logReachEvent(TransID transId, uint32_t guardIdx);

  // LoggingArray must keep the legacy bit for the wrapped array in sync.
  void setLegacyArrayInPlace(bool legacy);

  bool checkInvariants() const;

#define X(Return, Name, Args...) static Return Name(Args);
  BESPOKE_LAYOUT_FUNCTIONS
#undef X

  ArrayData* wrapped;
  LoggingProfile* profile;
  EntryTypes entryTypes;
};

}}

#endif // HPHP_LOGGING_ARRAY_H_
