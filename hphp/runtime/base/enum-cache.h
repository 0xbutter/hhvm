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

#ifndef incl_HPHP_ENUM_CACHE_H_
#define incl_HPHP_ENUM_CACHE_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/vm/class.h"
#include <tbb/concurrent_hash_map.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

// Values in the TBB map that contain the enum static arrays
struct EnumValues {
  // array from 'enum name' to 'enum value'
  // e.g. [ 'RED' => 1, 'BLUE' =>2, ...]
  Array values;
  // array from 'enum value' to 'enum name'
  // e.g. [ 1 => 'RED', 2 => 'BLUE', ...]
  Array names;
};

struct EnumCache {
  EnumCache() {}
  ~EnumCache();

  // TBB hash and compare struct
  struct clsCompare {
    bool equal(intptr_t key1, intptr_t key2) const {
      assertx(key1 && key2);
      bool equal = (key1 == key2);
      assertx(!equal || getClass(key1)->name()->equal(getClass(key2)->name()));
      return equal;
    }

    size_t hash(intptr_t key) const {
      assertx(key);
      return static_cast<size_t>(hash_int64(key));
    }
  };

  // if the class provided derives from Enum the name/value and value/name
  // arrays are build, stored in the cache and returned.
  // If not an error is raised.
  // If the recurse flag is 'true' array values are loaded up the hierarchy
  // chain (if any).
  static const EnumValues* getValues(const Class* klass, bool recurse);
  // Like above, but for first-class enums
  static const EnumValues* getValuesBuiltin(const Class* klass);
  // Like above, but for first-class enums where the values can be computed
  // entirely statically. Returns nullptr if that's not the case.
  static const EnumValues* getValuesStatic(const Class* klass);
  // delete the EnumValues element in the cache for the given class.
  // If there is no entry this function is a no-op.
  static void deleteValues(const Class* klass);

  // Helper that raises a PHP exception
  [[noreturn]] static void failLookup(const Variant& msg);

  // Large enums get the dummy LargeEnum tag (so that we can cache a single
  // static value for these enums). Small enums get a tag based on the caller.
  static Array tagEnumWithProvenance(Array input);

private:
  // Class* to intptr_ti key helpers
  const static intptr_t RECURSE_MASK = 1;
  static const Class* getClass(intptr_t key) {
    return reinterpret_cast<const Class*>(key & ~RECURSE_MASK);
  }

  static intptr_t getKey(const Class* klass, bool recurse) {
    intptr_t key = reinterpret_cast<intptr_t>(klass);
    return (recurse) ? key | RECURSE_MASK : key;
  }

  const EnumValues* cachePersistentEnumValues(
    const Class* klass,
    bool recurse,
    Array&& names,
    Array&& values);

  const EnumValues* cacheRequestEnumValues(
    const Class* klass,
    bool recurse,
    Array&& names,
    Array&& values);

  const EnumValues* getEnumValuesIfDefined(intptr_t key,
    bool checkLocal = true) const;
  const EnumValues* getEnumValues(const Class* klass, bool recurse,
                                  bool require_static = false);
  const EnumValues* loadEnumValues(const Class* klass, bool recurse,
                                   bool require_static = false);
  void deleteEnumValues(intptr_t key);

  // Map that contains associations between Enum classes and their array
  // values and array names.
  using EnumValuesMap = tbb::concurrent_hash_map<
    intptr_t,
    const EnumValues*,
    clsCompare>;

  using ReqEnumValuesMap = req::fast_map<
    intptr_t,
    const EnumValues*>;

  // Persistent values, recursive case. Non-recursive are cached in Class.
  EnumValuesMap m_enumValuesMap;

  rds::Link<ReqEnumValuesMap*, rds::Mode::Normal> m_nonScalarEnumValuesMap;
};

//////////////////////////////////////////////////////////////////////

}

#endif
