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
#include "hphp/runtime/vm/jit/irgen-call.h"

#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-create.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_self("self");
const StaticString s_parent("parent");
const StaticString s_static("static");

//////////////////////////////////////////////////////////////////////

const Func* findCuf(Op op,
                    SSATmp* callable,
                    const Class* ctx,
                    const Class*& cls,
                    StringData*& invName,
                    bool& forward) {
  cls = nullptr;
  invName = nullptr;

  const StringData* str =
    callable->isA(Type::Str) && callable->isConst() ? callable->strVal()
                                                    : nullptr;
  const ArrayData* arr =
    callable->isA(Type::Arr) && callable->isConst() ? callable->arrVal()
                                                    : nullptr;

  StringData* sclass = nullptr;
  StringData* sname = nullptr;
  if (str) {
    Func* f = Unit::lookupFunc(str);
    if (f) return f;
    String name(const_cast<StringData*>(str));
    int pos = name.find("::");
    if (pos <= 0 || pos + 2 >= name.size() ||
        name.find("::", pos + 2) != String::npos) {
      return nullptr;
    }
    sclass = makeStaticString(name.substr(0, pos).get());
    sname = makeStaticString(name.substr(pos + 2).get());
  } else if (arr) {
    if (arr->size() != 2) return nullptr;
    const Variant& e0 = arr->get(int64_t(0), false);
    const Variant& e1 = arr->get(int64_t(1), false);
    if (!e0.isString() || !e1.isString()) return nullptr;
    sclass = e0.getStringData();
    sname = e1.getStringData();
    String name(sname);
    if (name.find("::") != String::npos) return nullptr;
  } else {
    return nullptr;
  }

  if (sclass->isame(s_self.get())) {
    if (!ctx) return nullptr;
    cls = ctx;
    forward = true;
  } else if (sclass->isame(s_parent.get())) {
    if (!ctx || !ctx->parent()) return nullptr;
    cls = ctx->parent();
    forward = true;
  } else if (sclass->isame(s_static.get())) {
    return nullptr;
  } else {
    cls = Unit::lookupUniqueClass(sclass);
    if (!cls) return nullptr;
  }

  bool magicCall = false;
  const Func* f = lookupImmutableMethod(cls, sname, magicCall,
                                        /* staticLookup = */ true, ctx);
  if (!f || (forward && !ctx->classof(f->cls()))) {
    /*
     * To preserve the invariant that the lsb class
     * is an instance of the context class, we require
     * that f's class is an instance of the context class.
     * This is conservative, but without it, we would need
     * a runtime check to decide whether or not to forward
     * the lsb class
     */
    return nullptr;
  }
  if (magicCall) invName = sname;
  return f;
}

bool canInstantiateClass(const Class* cls) {
  return cls && isNormalClass(cls) && !isAbstract(cls);
}

//////////////////////////////////////////////////////////////////////

// Pushing for object method when we don't know the Func* statically.
void fpushObjMethodUnknown(HTS& env,
                           SSATmp* obj,
                           const StringData* methodName,
                           int32_t numParams,
                           bool shouldFatal,
                           SSATmp* extraSpill) {
  spillStack(env);
  fpushActRec(env,
              cns(env, Type::Nullptr),  // Will be set by LdObjMethod
              obj,
              numParams,
              nullptr);
  auto const actRec = spillStack(env);
  auto const objCls = gen(env, LdObjClass, obj);

  // This is special. We need to move the stackpointer in case
  // LdObjMethod calls a destructor. Otherwise it would clobber the
  // ActRec we just pushed.
  updateMarker(env);
  Block* catchBlock;
  if (extraSpill) {
    /*
     * If LdObjMethod throws, it nulls out the ActRec (since the unwinder
     * will attempt to destroy it as if it were cells), and then writes
     * obj into the last entry, since we need it to be destroyed.
     * If we have another object to destroy, we should write it in
     * the first - so pop 1 cell, then push extraSpill.
     */
    auto spill = std::vector<SSATmp*>{extraSpill};
    catchBlock = makeCatch(env, spill, 1);
  } else {
    catchBlock = makeCatchNoSpill(env);
  }
  gen(env,
      LdObjMethod,
      LdObjMethodData { methodName, shouldFatal },
      catchBlock,
      objCls,
      actRec);
}

void fpushObjMethodCommon(HTS& env,
                          SSATmp* obj,
                          const StringData* methodName,
                          int32_t numParams,
                          bool shouldFatal,
                          SSATmp* extraSpill) {
  SSATmp* objOrCls = obj;
  const Class* baseClass = nullptr;
  if (obj->type().isSpecialized()) {
    auto cls = obj->type().getClass();
    if (!env.irb->constrainValue(obj, TypeConstraint(cls).setWeak())) {
      // If we know the class without having to specialize a guard any further,
      // use it.
      baseClass = cls;
    }
  }

  bool magicCall = false;
  const Func* func = lookupImmutableMethod(baseClass,
                                           methodName,
                                           magicCall,
                                           /* staticLookup: */
                                           false,
                                           curClass(env));

  if (!func) {
    if (baseClass && !(baseClass->attrs() & AttrInterface)) {
      auto const res =
        g_context->lookupObjMethod(func, baseClass, methodName, curClass(env),
                                   false);
      if (res == LookupResult::MethodFoundWithThis ||
          /*
           * TODO(#4455926): We don't allow vtable-style dispatch of
           * abstract static methods, but not for any real reason
           * here.  It should be able to work, but needs further
           * testing to be enabled.
           */
          (res == LookupResult::MethodFoundNoThis && !func->isAbstract())) {
        /*
         * If we found the func in baseClass, then either:
         *  a) its private, and this is always going to be the
         *     called function. This case is handled further down.
         * OR
         *  b) any derived class must have a func that matches in staticness
         *     and is at least as accessible (and in particular, you can't
         *     override a public/protected method with a private method).  In
         *     this case, we emit code to dynamically lookup the method given
         *     the Object and the method slot, which is the same as func's.
         */
        if (!(func->attrs() & AttrPrivate)) {
          auto const clsTmp = gen(env, LdObjClass, obj);
          auto const funcTmp = gen(
            env,
            LdClsMethod,
            clsTmp,
            cns(env, -(func->methodSlot() + 1))
          );
          if (res == LookupResult::MethodFoundNoThis) {
            gen(env, DecRef, obj);
            objOrCls = clsTmp;
          }
          fpushActRec(env,
                      funcTmp,
                      objOrCls,
                      numParams,
                      magicCall ? methodName : nullptr);
          return;
        }
      } else {
        // method lookup did not find anything
        func = nullptr; // force lookup
      }
    }
  }

  if (func != nullptr) {
    /*
     * static function: store base class into this slot instead of obj
     * and decref the obj that was pushed as the this pointer since
     * the obj won't be in the actrec and thus MethodCache::lookup won't
     * decref it
     *
     * static closure body: we still need to pass the object instance
     * for the closure prologue to properly do its dispatch (and
     * extract use vars).  It will decref it and put the class on the
     * actrec before entering the "real" cloned closure body.
     */
    if (func->attrs() & AttrStatic && !func->isClosureBody()) {
      assert(baseClass);
      gen(env, DecRef, obj);
      objOrCls = cns(env, baseClass);
    }
    fpushActRec(env,
                cns(env, func),
                objOrCls,
                numParams,
                magicCall ? methodName : nullptr);
    return;
  }

  fpushObjMethodUnknown(env, obj, methodName, numParams, shouldFatal,
    extraSpill);
}

void fpushFuncObj(HTS& env, int32_t numParams) {
  auto const slowExit = makeExitSlow(env);
  auto const obj      = popC(env);
  auto const cls      = gen(env, LdObjClass, obj);
  auto const func     = gen(env, LdObjInvoke, slowExit, cls);
  fpushActRec(env, func, obj, numParams, nullptr);
}

void fpushFuncArr(HTS& env, int32_t numParams) {
  auto const thisAR = fp(env);

  auto const arr = popC(env);
  fpushActRec(
    env,
    cns(env, Type::Nullptr),
    cns(env, Type::Nullptr),
    numParams,
    nullptr
  );
  auto const actRec = spillStack(env);

  // This is special. We need to move the stackpointer incase LdArrFuncCtx
  // calls a destructor. Otherwise it would clobber the ActRec we just
  // pushed.
  updateMarker(env);

  gen(env, LdArrFuncCtx, makeCatch(env, {arr}, 1), arr, actRec, thisAR);
  gen(env, DecRef, arr);
}

bool fpushCufArray(HTS& env, SSATmp* callable, int32_t numParams) {
  if (!callable->isA(Type::Arr)) return false;

  auto callableInst = callable->inst();
  if (!callableInst->is(NewPackedArray)) return false;

  auto callableSize = callableInst->src(0);
  if (!callableSize->isConst() ||
      callableSize->intVal() != 2) {
    return false;
  }

  auto method = getStackValue(sp(env), 0).value;
  auto object = getStackValue(sp(env), 1).value;
  if (!method || !object) return false;

  if (!method->isConst(Type::Str) ||
      strstr(method->strVal()->data(), "::") != nullptr) {
    return false;
  }

  if (!object->isA(Type::Obj)) {
    if (!object->type().equals(Type::Cell)) return false;
    // This is probably an object, and we just haven't guarded on
    // the type.  Do so now.
    auto const exit = makeExit(env);
    object = gen(env, CheckType, Type::Obj, exit, object);
  }
  env.irb->constrainValue(object, DataTypeSpecific);

  popC(env);

  gen(env, IncRef, object);
  fpushObjMethodCommon(env,
                       object,
                       method->strVal(),
                       numParams,
                       false /* shouldFatal */,
                       callable);
  gen(env, DecRef, callable);
  return true;
}

// FPushCuf when the callee is not known at compile time.
void fpushCufUnknown(HTS& env, Op op, int32_t numParams) {
  if (op != Op::FPushCuf) {
    PUNT(fpushCufUnknown-nonFPushCuf);
  }

  if (topC(env)->isA(Type::Obj)) return fpushFuncObj(env, numParams);

  if (!topC(env)->type().subtypeOfAny(Type::Arr, Type::Str)) {
    PUNT(fpushCufUnknown);
  }

  // Peek at the top of the stack before deciding to pop it.
  auto const callable = topC(env);
  if (fpushCufArray(env, callable, numParams)) return;

  popC(env);

  fpushActRec(
    env,
    cns(env, Type::Nullptr),
    cns(env, Type::Nullptr),
    numParams,
    nullptr
  );
  auto const actRec = spillStack(env);

  /*
   * This is a similar case to lookup for functions in FPushFunc or
   * FPushObjMethod.  We can throw in a weird situation where the
   * ActRec is already on the stack, but this bytecode isn't done
   * executing yet.  See arPreliveOverwriteCells for details about why
   * we need this marker.
   */
  updateMarker(env);

  auto const opcode = callable->isA(Type::Arr) ? LdArrFPushCuf
                                               : LdStrFPushCuf;
  gen(env, opcode, makeCatch(env, {callable}, 1), callable, actRec, fp(env));
  gen(env, DecRef, callable);
}

SSATmp* clsMethodCtx(HTS& env, const Func* callee, const Class* cls) {
  bool mustBeStatic = true;

  if (!(callee->attrs() & AttrStatic) &&
      !(curFunc(env)->attrs() & AttrStatic) &&
      curClass(env)) {
    if (curClass(env)->classof(cls)) {
      // In this case, it might not be static, but we can be sure
      // we're going to forward $this if thisAvailable.
      mustBeStatic = false;
    } else if (cls->classof(curClass(env))) {
      // Unlike the above, we might be calling down to a subclass that
      // is not related to the current instance.  To know whether this
      // call forwards $this requires a runtime type check, so we have
      // to punt instead of trying the thisAvailable path below.
      PUNT(getClsMethodCtx-PossibleStaticRelatedCall);
    }
  }

  if (mustBeStatic) {
    return ldCls(env, makeCatch(env), cns(env, cls->name()));
  }
  if (env.irb->thisAvailable()) {
    // might not be a static call and $this is available, so we know it's
    // definitely not static
    assert(curClass(env));
    auto this_ = ldThis(env);
    gen(env, IncRef, this_);
    return this_;
  }
  // might be a non-static call. we have to inspect the func at runtime
  PUNT(getClsMethodCtx-MightNotBeStatic);
}

void implFPushCufOp(HTS& env, Op op, int32_t numArgs) {
  const bool safe = op == OpFPushCufSafe;
  bool forward = op == OpFPushCufF;
  SSATmp* callable = topC(env, safe ? 1 : 0);

  const Class* cls = nullptr;
  StringData* invName = nullptr;
  auto const callee = findCuf(op, callable, curClass(env), cls, invName,
    forward);
  if (!callee) return fpushCufUnknown(env, op, numArgs);

  SSATmp* ctx;
  auto const safeFlag = cns(env, true); // This is always true until the slow exits
                                        // below are implemented
  auto func = cns(env, callee);
  if (cls) {
    auto const exitSlow = makeExitSlow(env);
    if (!RDS::isPersistentHandle(cls->classHandle())) {
      // The miss path is complicated and rare.  Punt for now.  This must be
      // checked before we IncRef the context below, because the slow exit will
      // want to do that same IncRef via InterpOne.
      auto const clsOrNull = gen(env, LdClsCachedSafe, cns(env, cls->name()));
      gen(env, CheckNonNull, exitSlow, clsOrNull);
    }

    if (forward) {
      ctx = ldCtx(env);
      ctx = gen(env, GetCtxFwdCall, ctx, cns(env, callee));
    } else {
      ctx = clsMethodCtx(env, callee, cls);
    }
  } else {
    ctx = cns(env, Type::Nullptr);
    if (!RDS::isPersistentHandle(callee->funcHandle())) {
      // The miss path is complicated and rare. Punt for now.
      func = gen(env, LdFuncCachedSafe, LdFuncCachedData(callee->name()));
      func = gen(env, CheckNonNull, makeExitSlow(env), func);
    }
  }

  auto const defaultVal = safe ? popC(env) : nullptr;
  popDecRef(env, Type::Cell); // callable
  if (safe) {
    push(env, defaultVal);
    push(env, safeFlag);
  }

  fpushActRec(env, func, ctx, numArgs, invName);
}

void fpushCtorCommon(HTS& env,
                     SSATmp* cls,
                     SSATmp* obj,
                     const Func* func,
                     int32_t numParams) {
  push(env, obj);
  auto const fn = [&] {
    if (func) return cns(env, func);
    /*
      Without the updateMarker, the catch trace will write
      obj onto the stack, but the VMRegAnchor will setup the
      stack as it was before the FPushCtor*, which (for
      FPushCtorD at least) won't include obj
    */
    updateMarker(env);
    return gen(env, LdClsCtor, makeCatch(env), cls);
  }();
  gen(env, IncRef, obj);
  auto numArgsAndFlags = ActRec::encodeNumArgs(numParams, false, false, true);
  fpushActRec(env, fn, obj, numArgsAndFlags, nullptr);
}

void fpushFuncCommon(HTS& env,
                     const Func* func,
                     const StringData* name,
                     const StringData* fallback,
                     int32_t numParams) {
  if (func) {
    func->validate();
    if (func->isNameBindingImmutable(curUnit(env))) {
      fpushActRec(env,
                  cns(env, func),
                  cns(env, Type::Nullptr),
                  numParams,
                  nullptr);
      return;
    }
  }

  auto const catchBlock = makeCatch(env);
  auto const ssaFunc = fallback
    ? gen(env,
          LdFuncCachedU,
          LdFuncCachedUData { name, fallback },
          catchBlock)
    : gen(env,
          LdFuncCached,
          LdFuncCachedData { name },
          catchBlock);
  fpushActRec(env,
              ssaFunc,
              cns(env, Type::Nullptr),
              numParams,
              nullptr);
}

void implUnboxR(HTS& env) {
  auto const exit = makeExit(env);
  auto const srcBox = popR(env);
  auto const unboxed = unbox(env, srcBox, exit);
  if (unboxed == srcBox) {
    // If the Unbox ended up being a noop, don't bother refcounting
    push(env, unboxed);
  } else {
    pushIncRef(env, unboxed);
    gen(env, DecRef, srcBox);
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void fpushActRec(HTS& env,
                 SSATmp* func,
                 SSATmp* objOrClass,
                 int32_t numArgs,
                 const StringData* invName) {
  /*
   * Before allocating an ActRec, we do a spillStack so we'll have a
   * StkPtr that represents what the stack will look like after the
   * ActRec is popped.
   */
  auto const actualStack = spillStack(env);
  auto const returnSp = actualStack;

  env.fpiStack.emplace(returnSp, env.irb->spOffset());

  ActRecInfo info;
  info.numArgs = numArgs;
  info.invName = invName;
  gen(
    env,
    SpillFrame,
    info,
    // Using actualStack instead of returnSp so SpillFrame still gets
    // the src in rVmSp.  (TODO(#2288359).)
    actualStack,
    func,
    objOrClass
  );
  assert(env.irb->stackDeficit() == 0);
}

//////////////////////////////////////////////////////////////////////

void emitFPushCufIter(HTS& env, int32_t numParams, int32_t itId) {
  auto stack = spillStack(env);
  env.fpiStack.emplace(stack, env.irb->spOffset());
  gen(env,
      CufIterSpillFrame,
      FPushCufData(numParams, itId),
      stack,
      fp(env));
}

void emitFPushCuf(HTS& env, int32_t numArgs) {
  implFPushCufOp(env, Op::FPushCuf, numArgs);
}
void emitFPushCufF(HTS& env, int32_t numArgs) {
  implFPushCufOp(env, Op::FPushCufF, numArgs);
}
void emitFPushCufSafe(HTS& env, int32_t numArgs) {
  implFPushCufOp(env, Op::FPushCufSafe, numArgs);
}

void emitFPushCtor(HTS& env, int32_t numParams) {
  auto const catchBlock = makeCatch(env);
  auto const cls = popA(env);
  auto const obj = gen(env, AllocObj, catchBlock, cls);
  gen(env, IncRef, obj);
  fpushCtorCommon(env, cls, obj, nullptr, numParams);
}

void emitFPushCtorD(HTS& env,
                    int32_t numParams,
                    const StringData* className) {
  auto const cls = Unit::lookupUniqueClass(className);
  bool const uniqueCls = classIsUnique(cls);
  bool const persistentCls = classHasPersistentRDS(cls);
  bool const canInstantiate = canInstantiateClass(cls);
  bool const fastAlloc =
    persistentCls &&
    canInstantiate &&
    !cls->callsCustomInstanceInit();

  const Func* func = uniqueCls ? cls->getCtor() : nullptr;
  if (func && !(func->attrs() & AttrPublic)) {
    auto const ctx = curClass(env);
    if (!ctx) {
      func = nullptr;
    } else if (ctx != cls) {
      if ((func->attrs() & AttrPrivate) ||
        !(ctx->classof(cls) || cls->classof(ctx))) {
        func = nullptr;
      }
    }
  }

  auto ssaCls = persistentCls
    ? cns(env, cls)
    : gen(env, LdClsCached, makeCatch(env), cns(env, className));
  if (!ssaCls->isConst() && uniqueCls) {
    // If the Class is unique but not persistent, it's safe to use it as a
    // const after the LdClsCached, which will throw if the class can't be
    // defined.
    ssaCls = cns(env, cls);
  }

  auto const obj = fastAlloc
    ? allocObjFast(env, cls)
    : gen(env, AllocObj, makeCatch(env), ssaCls);
  gen(env, IncRef, obj);
  fpushCtorCommon(env, ssaCls, obj, func, numParams);
}

void emitFPushFuncD(HTS& env, int32_t numParams, const StringData* funcName) {
  auto const func = Unit::lookupFunc(funcName);
  fpushFuncCommon(env, func, funcName, nullptr, numParams);
}

void emitFPushFuncU(HTS& env,
                    int32_t numParams,
                    const StringData* funcName,
                    const StringData* fallbackName) {
  auto const func = Unit::lookupFunc(funcName);
  fpushFuncCommon(env, func, funcName, fallbackName, numParams);
}

void emitFPushFunc(HTS& env, int32_t numParams) {
  if (topC(env)->isA(Type::Obj)) return fpushFuncObj(env, numParams);
  if (topC(env)->isA(Type::Arr)) return fpushFuncArr(env, numParams);

  if (!topC(env)->isA(Type::Str)) {
    PUNT(FPushFunc_not_Str);
  }

  auto const catchBlock = makeCatch(env);
  auto const funcName = popC(env);
  fpushActRec(env,
              gen(env, LdFunc, catchBlock, funcName),
              cns(env, Type::Nullptr),
              numParams,
              nullptr);
}

void emitFPushObjMethodD(HTS& env,
                         int32_t numParams,
                         const StringData* methodName,
                         ObjMethodOp subop) {
  auto const obj = popC(env);
  if (!obj->isA(Type::Obj)) PUNT(FPushObjMethodD-nonObj);
  fpushObjMethodCommon(env, obj, methodName, numParams,
    true /* shouldFatal */, nullptr);
}

void emitFPushClsMethodD(HTS& env,
                         int32_t numParams,
                         const StringData* methodName,
                         const StringData* className) {
  auto const baseClass  = Unit::lookupUniqueClass(className);
  bool magicCall        = false;

  if (auto const func = lookupImmutableMethod(baseClass,
                                              methodName,
                                              magicCall,
                                              true /* staticLookup */,
                                              curClass(env))) {
    auto const objOrCls = clsMethodCtx(env, func, baseClass);
    fpushActRec(env,
                cns(env, func),
                objOrCls,
                numParams,
                func && magicCall ? methodName : nullptr);
    return;
  }

  auto const slowExit = makeExitSlow(env);
  auto const ne = NamedEntity::get(className);
  auto const data = ClsMethodData { className, methodName, ne };

  // Look up the Func* in the targetcache. If it's not there, try the slow
  // path. If that fails, slow exit.
  auto const func = env.irb->cond(
    0,
    [&] (Block* taken) {
      auto const mcFunc = gen(env, LdClsMethodCacheFunc, data);
      return gen(env, CheckNonNull, taken, mcFunc);
    },
    [&] (SSATmp* func) { // next
      return func;
    },
    [&] { // taken
      env.irb->hint(Block::Hint::Unlikely);
      auto result = gen(env,
                        LookupClsMethodCache,
                        makeCatch(env),
                        data,
                        fp(env));
      return gen(env, CheckNonNull, slowExit, result);
    }
  );
  auto const clsCtx = gen(env, LdClsMethodCacheCls, data);

  fpushActRec(env,
              func,
              clsCtx,
              numParams,
              nullptr);
}

void emitFPushClsMethod(HTS& env, int32_t numParams) {
  auto const clsVal  = popA(env);
  auto const methVal = popC(env);

  if (!methVal->isA(Type::Str) || !clsVal->isA(Type::Cls)) {
    PUNT(FPushClsMethod-unknownType);
  }

  if (methVal->isConst()) {
    const Class* cls = nullptr;
    if (clsVal->isConst()) {
      cls = clsVal->clsVal();
    } else if (clsVal->inst()->op() == LdClsCctx) {
      /*
       * Optimize FPushClsMethod when the method is a known static
       * string and the input class is the context.  The common bytecode
       * pattern here is LateBoundCls ; FPushClsMethod.
       *
       * This logic feels like it belongs in the simplifier, but the
       * generated code for this case is pretty different, since we
       * don't need the pre-live ActRec trick.
       */
      cls = curClass(env);
    }

    if (cls) {
      const Func* func;
      auto res =
        g_context->lookupClsMethod(func,
                                     cls,
                                     methVal->strVal(),
                                     nullptr,
                                     cls,
                                     false);
      if (res == LookupResult::MethodFoundNoThis && func->isStatic()) {
        auto funcTmp = clsVal->isConst()
          ? cns(env, func)
          : gen(env, LdClsMethod, clsVal, cns(env, -(func->methodSlot() + 1)));
        fpushActRec(env, funcTmp, clsVal, numParams, nullptr);
        return;
      }
    }
  }

  fpushActRec(env,
              cns(env, Type::Nullptr),
              cns(env, Type::Nullptr),
              numParams,
              nullptr);
  auto const actRec = spillStack(env);

  /*
   * Similar to FPushFunc/FPushObjMethod, we have an incomplete ActRec
   * on the stack and must handle that properly if we throw.
   */
  updateMarker(env);

  gen(env, LookupClsMethod, makeCatch(env, {methVal, clsVal}), clsVal,
    methVal, actRec, fp(env));
  gen(env, DecRef, methVal);
}

void emitFPushClsMethodF(HTS& env, int32_t numParams) {
  auto const exitBlock = makeExitSlow(env);

  auto classTmp = top(env, Type::Cls);
  auto methodTmp = topC(env, 1, DataTypeGeneric);
  assert(classTmp->isA(Type::Cls));
  if (!classTmp->isConst() || !methodTmp->isConst(Type::Str)) {
    PUNT(FPushClsMethodF-unknownClassOrMethod);
  }
  env.irb->constrainValue(methodTmp, DataTypeSpecific);

  auto const cls = classTmp->clsVal();
  auto const methName = methodTmp->strVal();

  bool magicCall = false;
  auto const vmfunc = lookupImmutableMethod(cls,
                                            methName,
                                            magicCall,
                                            true /* staticLookup */,
                                            curClass(env));
  auto const catchBlock = vmfunc ? nullptr : makeCatch(env);
  discard(env, 2);

  auto const curCtxTmp = ldCtx(env);
  if (vmfunc) {
    auto const funcTmp = cns(env, vmfunc);
    auto const newCtxTmp = gen(env, GetCtxFwdCall, curCtxTmp, funcTmp);
    fpushActRec(env, funcTmp, newCtxTmp, numParams,
      (magicCall ? methName : nullptr));
    return;
  }

  auto const data = ClsMethodData{cls->name(), methName};
  auto const funcTmp = env.irb->cond(
    0,
    [&](Block* taken) {
      auto const fcacheFunc = gen(env, LdClsMethodFCacheFunc, data);
      return gen(env, CheckNonNull, taken, fcacheFunc);
    },
    [&](SSATmp* func) { // next
      return func;
    },
    [&] { // taken
      env.irb->hint(Block::Hint::Unlikely);
      auto const result = gen(
        env,
        LookupClsMethodFCache,
        catchBlock,
        data,
        cns(env, cls),
        fp(env)
      );
      return gen(env, CheckNonNull, exitBlock, result);
    }
  );

  auto const ctx = gen(env, GetCtxFwdCallDyn, data, curCtxTmp);
  fpushActRec(env,
              funcTmp,
              ctx,
              numParams,
              magicCall ? methName : nullptr);
}

//////////////////////////////////////////////////////////////////////

void emitFPassL(HTS& env, int32_t argNum, int32_t id) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    emitVGetL(env, id);
  } else {
    emitCGetL(env, id);
  }
}

void emitFPassS(HTS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    emitVGetS(env);
  } else {
    emitCGetS(env);
  }
}

void emitFPassG(HTS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    emitVGetG(env);
  } else {
    emitCGetG(env);
  }
}

void emitFPassR(HTS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    PUNT(FPassR-byRef);
  }

  implUnboxR(env);
}

void emitUnboxR(HTS& env) { implUnboxR(env); }

void emitFPassV(HTS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    // FPassV is a no-op when the callee expects by ref.
    return;
  }

  auto const tmp = popV(env);
  pushIncRef(env, gen(env, LdRef, Type::InitCell, tmp));
  gen(env, DecRef, tmp);
}

void emitFPassCE(HTS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    // Need to raise an error
    PUNT(FPassCE-byRef);
  }
}

void emitFPassCW(HTS& env, int32_t argNum) {
  if (env.currentNormalizedInstruction->preppedByRef) {
    // Need to raise a warning
    PUNT(FPassCW-byRef);
  }
}

//////////////////////////////////////////////////////////////////////

void emitFCallArray(HTS& env) {
  auto const data = CallArrayData {
    bcOff(env),
    nextBcOff(env),
    callDestroysLocals(*env.currentNormalizedInstruction, curFunc(env))
  };
  auto const stack = spillStack(env);
  gen(env, CallArray, data, stack, fp(env));
}

void emitFCallD(HTS& env,
                int32_t numParams,
                const StringData*,
                const StringData*) {
  emitFCall(env, numParams);
}

void emitFCall(HTS& env, int32_t numParams) {
  auto const returnBcOffset = nextBcOff(env) - curFunc(env)->base();
  auto const callee = env.currentNormalizedInstruction->funcd;
  auto const destroyLocals = callDestroysLocals(
    *env.currentNormalizedInstruction,
    curFunc(env)
  );

  if (RuntimeOption::EvalRuntimeTypeProfile) {
    for (auto i = uint32_t{0}; i < numParams; ++i) {
      auto const val = topF(env, numParams - i - 1);
      if (callee != nullptr) {
        gen(env, TypeProfileFunc, TypeProfileData(i), val, cns(env, callee));
      } else  {
        auto const func = gen(env, LdARFuncPtr, sp(env), cns(env, 0));
        gen(env, TypeProfileFunc, TypeProfileData(i), val, func);
      }
    }
  }

  /*
   * Figure out if we know where we're going already (if a prologue was already
   * generated, we don't need to do a whole bind call thing again).
   *
   * We're skipping magic calls right now because 'callee' will be set to
   * __call in some cases (with 86ctor) where we shouldn't really call that
   * function (arguable bug in annotation).
   *
   * TODO(#4357498): This is currently disabled, because we haven't set things
   * up properly to be able to eagerly bind.  Because code-gen can punt, the
   * code there needs to delay adding these smash locations until after we know
   * the translation isn't punted.
   */
  auto const knownPrologue = [&]() -> TCA {
    if (false) {
      if (!callee || callee->isMagic()) return nullptr;
      auto const prologueIndex =
        numParams <= callee->numNonVariadicParams()
          ? numParams
          : callee->numNonVariadicParams() + 1;
      TCA ret;
      if (!mcg->checkCachedPrologue(callee, prologueIndex, ret)) {
        return nullptr;
      }
      return ret;
    }
    return nullptr;
  }();

  auto const stack = spillStack(env);
  gen(
    env,
    Call,
    CallData {
      static_cast<uint32_t>(numParams),
      returnBcOffset,
      callee,
      destroyLocals,
      knownPrologue
    },
    stack,
    fp(env)
  );

  if (!env.fpiStack.empty()) {
    env.fpiStack.pop();
  }
}

//////////////////////////////////////////////////////////////////////

}}}

