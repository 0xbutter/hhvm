/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/normalized-instruction.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

// Return a constant SSATmp representing a static value held in a TypedValue.
// The TypedValue may be a non-scalar, but it must have a static value.
SSATmp* staticTVCns(HTS& env, const TypedValue* tv) {
  switch (tv->m_type) {
    case KindOfNull:          return cns(env, Type::InitNull);
    case KindOfBoolean:       return cns(env, !!tv->m_data.num);
    case KindOfInt64:         return cns(env, tv->m_data.num);
    case KindOfDouble:        return cns(env, tv->m_data.dbl);
    case KindOfStaticString:
    case KindOfString:        return cns(env, tv->m_data.pstr);
    case KindOfArray:         return cns(env, tv->m_data.parr);

    case KindOfUninit:
    case KindOfObject:
    case KindOfResource:
    case KindOfRef:
    case KindOfClass:
      break;
  }
  always_assert(false);
}

void implCns(HTS& env,
             const StringData* name,
             const StringData* fallbackName,
             bool error) {
  assert(fallbackName == nullptr || !error);
  auto const cnsNameTmp = cns(env, name);
  auto const tv = Unit::lookupPersistentCns(name);
  SSATmp* result = nullptr;

  SSATmp* fallbackNameTmp = nullptr;
  if (fallbackName != nullptr) {
    fallbackNameTmp = cns(env, fallbackName);
  }
  if (tv) {
    if (tv->m_type == KindOfUninit) {
      // KindOfUninit is a dynamic system constant. always a slow
      // lookup.
      assert(!fallbackNameTmp);
      if (error) {
        result = gen(env, LookupCnsE, makeCatch(env), cnsNameTmp);
      } else {
        result = gen(env, LookupCns, makeCatch(env), cnsNameTmp);
      }
    } else {
      result = staticTVCns(env, tv);
    }
  } else {
    auto const c1 = gen(env, LdCns, cnsNameTmp);
    result = env.irb->cond(
      1,
      [&] (Block* taken) { // branch
        gen(env, CheckInit, taken, c1);
      },
      [&] { // Next: LdCns hit in TC
        return c1;
      },
      [&] { // Taken: miss in TC, do lookup & init
        env.irb->hint(Block::Hint::Unlikely);
        // We know that c1 is Uninit in this branch but we have to encode this
        // in the IR.
        gen(env, AssertType, Type::Uninit, c1);

        if (fallbackNameTmp) {
          return gen(env,
                     LookupCnsU,
                     makeCatch(env),
                     cnsNameTmp,
                     fallbackNameTmp);
        }
        if (error) {
          return gen(env, LookupCnsE, makeCatch(env), cnsNameTmp);
        }
        return gen(env, LookupCns, makeCatch(env), cnsNameTmp);
      }
    );
  }
  push(env, result);
}

//////////////////////////////////////////////////////////////////////

}

void emitCns(HTS& env, const StringData* name) {
  implCns(env, name, nullptr, false);
}

void emitCnsE(HTS& env, const StringData* name) {
  implCns(env, name, nullptr, true);
}

void emitCnsU(HTS& env,
              const StringData* name,
              const StringData* fallback) {
  implCns(env, name, fallback, false);
}

void emitClsCnsD(HTS& env,
                 const StringData* cnsNameStr,
                 const StringData* clsNameStr) {
  auto const outPred = env.currentNormalizedInstruction->outPred; // TODO: rm
  auto const clsCnsName = ClsCnsName { clsNameStr, cnsNameStr };

  // If we have to side exit, do the RDS lookup before chaining to
  // another Tracelet so forward progress still happens.
  auto catchBlock = makeCatchNoSpill(env);
  auto const sideExit = makeSideExit(
    env,
    nextBcOff(env),
    [&] {
      return gen(env, LookupClsCns, catchBlock, clsCnsName);
    }
  );

  /*
   * If the class is already defined in this request, the class is persistent
   * or a parent of the current context, and this constant is a scalar
   * constant, we can just compile it to a literal.
   */
  if (auto const cls = Unit::lookupClass(clsNameStr)) {
    Slot ignore;
    auto const tv = cls->cnsNameToTV(cnsNameStr, ignore);
    if (tv && tv->m_type != KindOfUninit &&
        classIsPersistentOrCtxParent(env, cls)) {
      push(env, staticTVCns(env, tv));
      return;
    }
  }

  /*
   * Otherwise, load the constant out of RDS.  Right now we always guard that
   * it is at least uncounted (this means a constant set to STDIN or something
   * will always side exit here).
   */
  auto const link = RDS::bindClassConstant(clsNameStr, cnsNameStr);
  auto const prds = gen(
    env,
    LdRDSAddr,
    RDSHandleData { link.handle() },
    Type::Cell.ptr(Ptr::ClsCns)
  );
  auto const guardType = outPred < Type::UncountedInit ? outPred
                                                       : Type::UncountedInit;
  gen(env, CheckTypeMem, guardType, sideExit, prds);
  push(env, gen(env, LdMem, guardType, prds, cns(env, 0)));
}

//////////////////////////////////////////////////////////////////////

}}}

