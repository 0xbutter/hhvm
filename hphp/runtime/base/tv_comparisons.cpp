/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/tv_comparisons.h"

#include <type_traits>

#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/type_conversions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

extern bool collectionEquals(ObjectData*, ObjectData*);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

bool cellToBool(const Cell* cell) {
  assert(tvIsPlausible(cell));
  assert(cell->m_type != KindOfRef);

  switch (cell->m_type) {
  case KindOfUninit:
  case KindOfNull:          return false;
  case KindOfInt64:         return cell->m_data.num != 0;
  case KindOfBoolean:       return cell->m_data.num;
  case KindOfDouble:        return cell->m_data.dbl != 0;
  case KindOfStaticString:
  case KindOfString:        return cell->m_data.pstr->toBoolean();
  case KindOfArray:         return !cell->m_data.parr->empty();
  case KindOfObject:        // TODO: should handle o_toBoolean?
                            return true;
  default:                  break;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

/*
 * Family of relative op functions.
 *
 * These are used to implement the common parts of the php operators
 * ==, <, and >.  They handle some of the php behavior with regard to
 * numeric-ish strings, and delegate to the 'op' functor to perform
 * the actual comparison on primitive types, and between complex php
 * types of the same type.
 *
 * See below for the implementations of the Op template parameter.
 */

template<class Op>
bool cellRelOp(Op op, const Cell* cell, bool val) {
  return op(cellToBool(cell), val);
}

template<class Op>
bool cellRelOp(Op op, const Cell* cell, int64_t val) {
  assert(tvIsPlausible(cell));
  assert(cell->m_type != KindOfRef);

  switch (cell->m_type) {
  case KindOfUninit:
  case KindOfNull:           return op(false, !!val);
  case KindOfBoolean:        return op(!!cell->m_data.num, val != 0);
  case KindOfInt64:          return op(cell->m_data.num, val);
  case KindOfDouble:         return op(cell->m_data.dbl, val);
  case KindOfArray:          return op(true, false);

  case KindOfObject:
    return cell->m_data.pobj->isCollection()
      ? op.collectionVsNonObj()
      : op(cell->m_data.pobj->o_toInt64(), val);

  case KindOfStaticString:
  case KindOfString:
    {
      auto const sdata = cell->m_data.pstr;
      int64_t ival;
      double dval;
      auto const dt = sdata->isNumericWithVal(ival, dval,
        /* allow_error */ true);
      return dt == KindOfInt64 ? op(ival, val) :
             dt == KindOfDouble ? op(dval, val) :
             op(0, val);
    }

  default:
    break;
  }
  not_reached();
}

template<class Op>
bool cellRelOp(Op op, const Cell* cell, double val) {
  assert(tvIsPlausible(cell));
  assert(cell->m_type != KindOfRef);

  switch (cell->m_type) {
  case KindOfUninit:
  case KindOfNull:            return op(false, val != 0);
  case KindOfBoolean:         return op(!!cell->m_data.num, val != 0);
  case KindOfInt64:           return op(cell->m_data.num, val);
  case KindOfDouble:          return op(cell->m_data.dbl, val);
  case KindOfArray:           return op(true, false);

  case KindOfObject:
    return cell->m_data.pobj->isCollection()
      ? op.collectionVsNonObj()
      : op(cell->m_data.pobj->o_toDouble(), val);

  case KindOfStaticString:
  case KindOfString:
    return op(toDouble(cell->m_data.pstr), val);

  default:
    break;
  }
  not_reached();
}

template<class Op>
bool cellRelOp(Op op, const Cell* cell, const StringData* val) {
  assert(tvIsPlausible(cell));
  assert(cell->m_type != KindOfRef);

  switch (cell->m_type) {
  case KindOfUninit:
  case KindOfNull:         return op(empty_string.get(), val);
  case KindOfBoolean:      return op(!!cell->m_data.num, toBoolean(val));
  case KindOfDouble:       return op(cell->m_data.dbl, val->toDouble());
  case KindOfArray:        return op(true, false);
  case KindOfString:
  case KindOfStaticString: return op(cell->m_data.pstr, val);

  case KindOfInt64:
    {
      int64_t ival;
      double dval;
      auto const dt = val->isNumericWithVal(ival, dval,
        /* allow_error */ true);
      return dt == KindOfInt64 ? op(cell->m_data.num, ival) :
             dt == KindOfDouble ? op(cell->m_data.num, dval) :
             op(cell->m_data.num, 0);
    }

  case KindOfObject:
    {
      auto const od = cell->m_data.pobj;
      if (od->isResource()) return op(true, false);
      if (od->isCollection()) return op.collectionVsNonObj();
      try {
        String str(const_cast<ObjectData*>(od)->t___tostring());
        return op(str.get(), val);
      } catch (BadTypeConversionException&) {
        return op(true, false);
      }
    }

  default:
    break;
  }
  not_reached();
}

template<class Op>
bool cellRelOp(Op op, const Cell* cell, const ArrayData* ad) {
  assert(tvIsPlausible(cell));
  assert(cell->m_type != KindOfRef);

  switch (cell->m_type) {
  case KindOfUninit:
  case KindOfNull:         return op(false, !ad->empty());
  case KindOfBoolean:      return op(cell->m_data.num, !ad->empty());
  case KindOfInt64:        return op(false, true);
  case KindOfDouble:       return op(false, true);
  case KindOfArray:        return op(cell->m_data.parr, ad);
  case KindOfStaticString:
  case KindOfString:       return op(false, true);
  case KindOfObject:
    return cell->m_data.pobj->isCollection()
      ? op.collectionVsNonObj()
      : op(true, false);

  default:
    break;
  }

  not_reached();
}

template<class Op>
bool cellRelOp(Op op, const Cell* cell, const ObjectData* od) {
  assert(tvIsPlausible(cell));
  assert(cell->m_type != KindOfRef);

  switch (cell->m_type) {
  case KindOfUninit:
  case KindOfNull:        // TODO: should use o_toBoolean
                          return op(false, true);
  case KindOfBoolean:     return op(!!cell->m_data.num, true);
  case KindOfInt64:
    return od->isCollection() ? op.collectionVsNonObj()
                              : op(cell->m_data.num, od->o_toInt64());
  case KindOfDouble:
    return od->isCollection() ? op.collectionVsNonObj()
                              : op(cell->m_data.dbl, od->o_toDouble());
  case KindOfArray:
    return od->isCollection() ? op.collectionVsNonObj() : op(false, true);

  case KindOfString:
  case KindOfStaticString:
    if (od->isResource()) return op(false, true);
    if (od->isCollection()) return op.collectionVsNonObj();
    try {
      String str(const_cast<ObjectData*>(od)->t___tostring());
      return op(cell->m_data.pstr, str.get());
    } catch (BadTypeConversionException&) {
      return op(false, true);
    }

  case KindOfObject:
    return op(cell->m_data.pobj, od);

  default:
    break;
  }

  not_reached();
}

template<class Op>
bool tvRelOp(Op op, const TypedValue* tv1, const TypedValue* tv2) {
  assert(tvIsPlausible(tv1));
  assert(tvIsPlausible(tv2));
  tv1 = tvToCell(tv1);
  tv2 = tvToCell(tv2);

  switch (tv2->m_type) {
  case KindOfUninit:
  case KindOfNull:
    return IS_STRING_TYPE(tv1->m_type)
      ? op(tv1->m_data.pstr, empty_string.get())
      : cellRelOp(op, tv1, false);
  case KindOfInt64:        return cellRelOp(op, tv1, tv2->m_data.num);
  case KindOfBoolean:      return cellRelOp(op, tv1, !!tv2->m_data.num);
  case KindOfDouble:       return cellRelOp(op, tv1, tv2->m_data.dbl);
  case KindOfStaticString:
  case KindOfString:       return cellRelOp(op, tv1, tv2->m_data.pstr);
  case KindOfArray:        return cellRelOp(op, tv1, tv2->m_data.parr);
  case KindOfObject:       return cellRelOp(op, tv1, tv2->m_data.pobj);
  default:
    break;
  }
  not_reached();
}

/*
 * These relative ops helper function objects define operator() for
 * each primitive type, and for the case of a complex type being
 * compared with itself (that is obj with obj, string with string,
 * array with array).
 *
 * They must also define a function called collectionVsNonObj() which
 * is used when comparing collections with non-object types.  (The obj
 * vs obj function should handle the collection vs collection and
 * collection vs non-collection object cases.)  This is just to handle
 * that php operator == returns false in these cases, while the Lt/Gt
 * operators throw and exception.
 */

struct Eq {
  template<class T, class U>
  typename std::enable_if<
    !std::is_pointer<T>::value &&
    !std::is_pointer<U>::value,
    bool
  >::type operator()(T t, U u) const { return t == u; }

  bool operator()(const StringData* sd1, const StringData* sd2) const {
    return sd1->equal(sd2);
  }

  bool operator()(const ArrayData* ad1, const ArrayData* ad2) const {
    return ad1->equal(ad2, false);
  }

  bool operator()(const ObjectData* od1, const ObjectData* od2) const {
    if (od1 == od2) return true;
    if (od1->isResource() || od2->isResource()) return false;
    if (od1->getVMClass() != od2->getVMClass()) return false;
    if (od1->isCollection()) {
      // TODO constness
      return collectionEquals(const_cast<ObjectData*>(od1),
                              const_cast<ObjectData*>(od2));
    }
    Array ar1(od1->o_toArray());
    Array ar2(od2->o_toArray());
    return ar1->equal(ar2.get(), false);
  }

  bool collectionVsNonObj() const { return false; }
};

struct Lt {
  template<class T, class U>
  typename std::enable_if<
    !std::is_pointer<T>::value &&
    !std::is_pointer<U>::value,
    bool
  >::type operator()(T t, U u) const { return t < u; }

  bool operator()(const StringData* sd1, const StringData* sd2) const {
    return sd1->compare(sd2) < 0;
  }

  bool operator()(const ArrayData* ad1, const ArrayData* ad2) const {
    return ad1->compare(ad2) < 0;
  }

  bool operator()(const ObjectData* od1, const ObjectData* od2) const {
    if (od1->isCollection() || od2->isCollection()) {
      throw_collection_compare_exception();
    }
    if (od1 == od2) return false;
    Array ar1(od1->o_toArray());
    Array ar2(od2->o_toArray());
    return (*this)(ar1.get(), ar2.get());
  }

  bool collectionVsNonObj() const {
    throw_collection_compare_exception();
    not_reached();
  }
};

struct Gt {
  template<class T, class U>
  typename std::enable_if<
    !std::is_pointer<T>::value &&
    !std::is_pointer<U>::value,
    bool
  >::type operator()(T t, U u) const { return t > u; }

  bool operator()(const StringData* sd1, const StringData* sd2) const {
    return sd1->compare(sd2) > 0;
  }

  bool operator()(const ArrayData* ad1, const ArrayData* ad2) const {
    return 0 > ad2->compare(ad1); // Not symmetric; order matters here.
  }

  bool operator()(const ObjectData* od1, const ObjectData* od2) const {
    if (od1->isCollection() || od2->isCollection()) {
      throw_collection_compare_exception();
    }
    if (od1 == od2) return false;
    Array ar1(od1->o_toArray());
    Array ar2(od2->o_toArray());
    return (*this)(ar1.get(), ar2.get());
  }

  bool collectionVsNonObj() const {
    throw_collection_compare_exception();
    not_reached();
  }
};

//////////////////////////////////////////////////////////////////////

}

bool tvSame(const TypedValue* tv1, const TypedValue* tv2) {
  assert(tvIsPlausible(tv1));
  assert(tvIsPlausible(tv2));

  tv1 = tvToCell(tv1);
  tv2 = tvToCell(tv2);

  bool const null1 = IS_NULL_TYPE(tv1->m_type);
  bool const null2 = IS_NULL_TYPE(tv2->m_type);
  if (null1 && null2) return true;
  if (null1 || null2) return false;

  switch (tv1->m_type) {
  case KindOfInt64:
  case KindOfBoolean:
    if (tv2->m_type != tv1->m_type) return false;
    return tv1->m_data.num == tv2->m_data.num;
  case KindOfDouble:
    if (tv2->m_type != tv1->m_type) return false;
    return tv1->m_data.dbl == tv2->m_data.dbl;

  case KindOfStaticString:
  case KindOfString:
    if (!IS_STRING_TYPE(tv2->m_type)) return false;
    return tv1->m_data.pstr->same(tv2->m_data.pstr);

  case KindOfArray:
    if (tv2->m_type != KindOfArray) return false;
    return tv1->m_data.parr->equal(tv2->m_data.parr, true);

  case KindOfObject:
    return tv2->m_type == KindOfObject &&
      tv1->m_data.pobj == tv2->m_data.pobj;

  default:
    break;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

/*
 * XXX: HOT_FUNC selections are basically whatever random choices were
 * in the old code ... we should probably re-evaluate this.
 */

bool cellEqual(const Cell* cell, bool val) {
  return cellRelOp(Eq(), cell, val);
}

HOT_FUNC
bool cellEqual(const Cell* cell, int64_t val) {
  return cellRelOp(Eq(), cell, val);
}

bool cellEqual(const Cell* cell, double val) {
  return cellRelOp(Eq(), cell, val);
}

bool cellEqual(const Cell* cell, const StringData* val) {
  return cellRelOp(Eq(), cell, val);
}

bool cellEqual(const Cell* cell, const ArrayData* val) {
  return cellRelOp(Eq(), cell, val);
}

bool cellEqual(const Cell* cell, const ObjectData* val) {
  return cellRelOp(Eq(), cell, val);
}

HOT_FUNC
bool tvEqual(const TypedValue* tv1, const TypedValue* tv2) {
  return tvRelOp(Eq(), tv1, tv2);
}

bool cellLess(const Cell* cell, bool val) {
  return cellRelOp(Lt(), cell, val);
}

bool cellLess(const Cell* cell, int64_t val) {
  return cellRelOp(Lt(), cell, val);
}

bool cellLess(const Cell* cell, double val) {
  return cellRelOp(Lt(), cell, val);
}

bool cellLess(const Cell* cell, const StringData* val) {
  return cellRelOp(Lt(), cell, val);
}

bool cellLess(const Cell* cell, const ArrayData* val) {
  return cellRelOp(Lt(), cell, val);
}

bool cellLess(const Cell* cell, const ObjectData* val) {
  return cellRelOp(Lt(), cell, val);
}

HOT_FUNC
bool tvLess(const TypedValue* tv1, const TypedValue* tv2) {
  return tvRelOp(Lt(), tv1, tv2);
}

bool cellGreater(const Cell* cell, bool val) {
  return cellRelOp(Gt(), cell, val);
}

//NB: was HOT_FUNC in old code ... dunno if this makes sense anymore.
bool cellGreater(const Cell* cell, int64_t val) {
  return cellRelOp(Gt(), cell, val);
}

bool cellGreater(const Cell* cell, double val) {
  return cellRelOp(Gt(), cell, val);
}

bool cellGreater(const Cell* cell, const StringData* val) {
  return cellRelOp(Gt(), cell, val);
}

bool cellGreater(const Cell* cell, const ArrayData* val) {
  return cellRelOp(Gt(), cell, val);
}

bool cellGreater(const Cell* cell, const ObjectData* val) {
  return cellRelOp(Gt(), cell, val);
}

bool tvGreater(const TypedValue* tv1, const TypedValue* tv2) {
  return tvRelOp(Gt(), tv1, tv2);
}

//////////////////////////////////////////////////////////////////////

}

