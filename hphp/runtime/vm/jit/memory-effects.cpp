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
#include "hphp/runtime/vm/jit/memory-effects.h"

#include "hphp/util/match.h"
#include "hphp/util/safe-cast.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/analysis.h"

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

AliasClass pointee(const SSATmp* ptr) {
  always_assert(ptr->type().isPtr());

  if (ptr->type() <= Type::PtrToFrameGen) {
    auto const sinst = canonical(ptr)->inst();
    if (sinst->is(LdLocAddr)) {
      return AFrame { sinst->src(0), sinst->extra<LdLocAddr>()->locId };
    }
    return AFrameAny;
  }

  if (ptr->type() <= Type::PtrToStkGen) {
    auto const sinst = canonical(ptr)->inst();
    if (sinst->is(LdStackAddr)) {
      return AStack { sinst->src(0), sinst->extra<LdStackAddr>()->offset, 1 };
    }
    return AStackAny;
  }

  if (ptr->type() <= Type::PtrToPropGen) {
    auto const sinst = canonical(ptr)->inst();
    if (sinst->is(LdPropAddr)) {
      return AProp {
        sinst->src(0),
        safe_cast<uint32_t>(sinst->src(1)->intVal())
      };
    }
    return APropAny;
  }

  // We have various other situations here that we don't track in this module
  // yet, but we can possibly exclude the locations we care about so far.
  auto const pty = ptr->type();
  if (!pty.maybe(Type::PtrToStkGen) && !pty.maybe(Type::PtrToFrameGen)) {
    return AHeapAny;
  }
  if (!pty.maybe(Type::PtrToFrameGen)) return ANonFrame;
  if (!pty.maybe(Type::PtrToStkGen)) return ANonStack;

  return AUnknown;
}

// Return an AliasClass representing a range of the eval stack that contains
// everything below a logical depth.
AliasClass stack_below(SSATmp* base, int32_t offset) {
  return AStack { base, offset, std::numeric_limits<int32_t>::max() };
}

bool call_destroys_locals(const IRInstruction& inst) {
  switch (inst.op()) {
  case Call:         return inst.extra<Call>()->destroyLocals;
  case CallArray:    return inst.extra<CallArray>()->destroyLocals;
  case CallBuiltin:  return inst.extra<CallBuiltin>()->destroyLocals;
  case ContEnter:    return false;
  default:           break;
  }
  always_assert(0);
}

/*
 * Returns an AliasClass that must be unioned into the may-load set of any
 * instruction that can re-enter the VM.  This set is empty if
 * EnableArgsInBacktraces is off---when it's on, in general re-entry could lead
 * to a call to debug_backtrace which could read the argument locals of any
 * activation in the callstack.
 *
 * We don't try to limit the effects to argument locals, though, and just union
 * in all the locals.
 *
 * This is unioned in in general when an instruction can re-enter because it
 * also makes that somewhat more obvious.
 */
AliasClass reentry_extra() {
  return RuntimeOption::EnableArgsInBacktraces ? AFrameAny : AEmpty;
}

//////////////////////////////////////////////////////////////////////

MemEffects memory_effects_impl(const IRInstruction& inst) {
  switch (inst.op()) {

  //////////////////////////////////////////////////////////////////////
  // Region exits

  // These exits don't leave the current php function, and could head to code
  // that could read or write anything as far as we know (including frame
  // locals).
  case ReqBindJmp:
    return ExitEffects { AUnknown, stack_below(inst.src(0), -1) };
  case JmpSwitchDest:
  case JmpSSwitchDest:
    return ExitEffects { AUnknown, stack_below(inst.src(1), -1) };
  case ReqRetranslate:
  case ReqRetranslateOpt:
    return UnknownEffects {};

  //////////////////////////////////////////////////////////////////////
  // Unusual instructions

  /*
   * The ReturnHook sets up the ActRec so the unwinder knows everything is
   * already released (i.e. it calls ar->setLocalsDecRefd()).
   *
   * So it has no upward exposed uses of locals, even though it has a catch
   * block as a successor that looks like it can use any locals (and in fact it
   * can, if it weren't for this instruction).
   */
  case ReturnHook:
    return KillFrameLocals { inst.src(0) };

  // The suspend hooks can load anything (re-entering the VM), but can't write
  // to frame locals.
  case SuspendHookE:
  case SuspendHookR:
    // TODO: may-load here probably doesn't need to include AFrameAny normally.
    return MayLoadStore { AUnknown | reentry_extra(), ANonFrame };

  /*
   * If we're returning from a function, it's ReturnEffects.  The RetCtrl
   * opcode also suspends resumables, which we model as having any possible
   * effects.
   */
  case RetCtrl:
    if (inst.extra<RetCtrl>()->suspendingResumed) {
      // Suspending can go anywhere, and doesn't even kill locals.
      return UnknownEffects {};
    }
    return ReturnEffects {
      stack_below(inst.src(0), inst.extra<RetCtrl>()->spOffset - 1)
    };

  case GenericRetDecRefs:
    return MayLoadStore { AUnknown, ANonFrame };

  case EndCatch:
    return ExitEffects {
      AUnknown,
      stack_below(inst.src(1), inst.extra<EndCatch>()->offset - 1)
    };

  /*
   * This has to act as a use of the locals for the outer frame, but really
   * only because of how we deal with FramePtrs in the JIT.
   *
   * Since the frame is going to get allocated rbp, we can't push local stores
   * into inlined callees, since their FramePtr isn't available anymore.  We'd
   * be willing to do so without this, since there's no other constraint saying
   * the caller's locals have to be in memory at this point.
   */
  case DefInlineFP:
    /*
     * TODO(#3634984): this is an unfortunate pessimization---we can still
     * eliminate those stores if nothing reads them, we just can't move them
     * past this point.  Reserved registers must die.
     */
    return MayLoadStore { AFrameAny, AEmpty };

  case InlineReturn:
    return KillFrameLocals { inst.src(0) };

  case InterpOne:
  case InterpOneCF:
    return InterpOneEffects {
      stack_below(inst.src(1), -inst.marker().spOff() - 1)
    };

  case NativeImpl:
    return UnknownEffects {};

  // NB: on the failure path, these C++ helpers do a fixup and read frame
  // locals before they throw.  They can also invoke the user error handler and
  // go do whatever they want to non-frame locations.
  //
  // TODO(#5372569): if we combine dv inits into the same regions we could
  // possibly avoid storing KindOfUninits if we modify this.
  case VerifyParamCallable:
  case VerifyParamCls:
  case VerifyParamFail:
    return MayLoadStore { AUnknown, ANonFrame };
  // However the following ones can't read locals from our frame on the way
  // out.
  case VerifyRetCallable:
  case VerifyRetCls:
  case VerifyRetFail:
    return MayLoadStore { AHeapAny | reentry_extra(), ANonFrame };

  case Call:
    return CallEffects {
      call_destroys_locals(inst),
      stack_below(inst.src(0), inst.extra<Call>()->spOffset - 1)
    };
  case CallArray:
  case CallBuiltin:
    // Note: CallBuiltin can have PtrToStkGen args that it writes/reads.  Right
    // now we don't need to explicitly handle this because we treat it as
    // may-load and may-store anything in both store-elim and load-elim,
    // respectively.
  case ContEnter:
    return CallEffects { call_destroys_locals(inst), AEmpty };

  // Resumable suspension takes everything from the frame and moves it into the
  // heap.
  case CreateAFWH:
  case CreateCont:
    return MayLoadStore { AFrameAny, ANonFrame };

  // This re-enters to call extension-defined instance constructors.
  case ConstructInstance:
    return MayLoadStore { AHeapAny | reentry_extra(), ANonFrame };

  //////////////////////////////////////////////////////////////////////
  // Iterator instructions

  case IterInit:
  case MIterInit:
  case WIterInit:
    return IterEffects {
      inst.src(1),
      inst.extra<IterData>()->valId,
      stack_below(inst.src(1), -inst.marker().spOff() - 1)
    };
  case IterNext:
  case MIterNext:
  case WIterNext:
    return IterEffects {
      inst.src(0),
      inst.extra<IterData>()->valId,
      stack_below(inst.src(0), -inst.marker().spOff() - 1)
    };

  case IterInitK:
  case MIterInitK:
  case WIterInitK:
    return IterEffects2 {
      inst.src(1),
      inst.extra<IterData>()->keyId,
      inst.extra<IterData>()->valId,
      stack_below(inst.src(1), -inst.marker().spOff() - 1)
    };

  case IterNextK:
  case MIterNextK:
  case WIterNextK:
    return IterEffects2 {
      inst.src(0),
      inst.extra<IterData>()->keyId,
      inst.extra<IterData>()->valId,
      stack_below(inst.src(0), -inst.marker().spOff() - 1)
    };

  //////////////////////////////////////////////////////////////////////
  // Instructions that explicitly manipulate locals

  case StLoc:
    return PureStore {
      AFrame { inst.src(0), inst.extra<StLoc>()->locId },
      inst.src(1)
    };

  case StLocNT:
    return PureStoreNT {
      AFrame { inst.src(0), inst.extra<StLocNT>()->locId },
      inst.src(1)
    };

  case LdLoc:
    return PureLoad { AFrame { inst.src(0), inst.extra<LocalId>()->locId } };

  case CheckLoc:
  case GuardLoc:
  case LdLocPseudoMain:
    return MayLoadStore {
      AFrame { inst.src(0), inst.extra<LocalId>()->locId },
      AEmpty
    };

  case StLocPseudoMain:
    // This can store to globals or locals, but we don't have globals supported
    // in AliasClass yet.
    return PureStore { AUnknown };

  case ClosureStaticLocInit:
    return MayLoadStore { AFrameAny, AFrameAny };

  //////////////////////////////////////////////////////////////////////
  // Pointer-based loads and stores

  case LdElem:
  case LdMem:
    return PureLoad { pointee(inst.src(0)) };

  case StElem:
  case StMem:
    return PureStore { pointee(inst.src(0)), inst.src(2) };

  case BoxPtr:
  case UnboxPtr:
    {
      auto const mem = pointee(inst.src(0));
      return MayLoadStore { mem, mem };
    }

  case CheckInitMem:
  case CheckTypeMem:
  case DbgAssertPtr:
  case IsNTypeMem:
  case IsTypeMem:
  case ProfileStr:
    return MayLoadStore { pointee(inst.src(0)), AEmpty };

  case DecRefMem:
    return MayLoadStore {
      // DecRefMem can re-enter to run a destructor.  We also need to union in
      // the pointee because it may point to a non-heap location.
      pointee(inst.src(0)) | AHeapAny | reentry_extra(),
      ANonFrame
    };

  //////////////////////////////////////////////////////////////////////
  // Object/Ref loads/stores

  case StProp:
    return PureStore {
      AProp { inst.src(0), safe_cast<uint32_t>(inst.src(1)->intVal()) },
      inst.src(2)
    };

  case CheckRefInner:
    // We don't have AliasClass support for refs yet, so it's a load from an
    // unknown heap location.
    return MayLoadStore { AHeapAny, AEmpty };
  case LdRef:
    return PureLoad { AHeapAny };

  case StRef:
    // We don't have anything for ref locations at this point, but we know it
    // is a heap location.
    return PureStore { AHeapAny };

  case InitObjProps:
    return MayLoadStore { AEmpty, APropAny };

  //////////////////////////////////////////////////////////////////////
  // Array loads and stores

  case InitPackedArray:
    return PureStore {
      AElemI { inst.src(0), inst.extra<InitPackedArray>()->index },
      inst.src(1)
    };

  // TODO(#5575265): use LdMem for this instruction.
  case LdPackedArrayElem:
    if (inst.src(1)->isConst() && inst.src(1)->intVal() >= 0) {
      return PureLoad {
        AElemI { inst.src(0), safe_cast<uint64_t>(inst.src(1)->intVal()) }
      };
    }
    return PureLoad { AElemIAny };

  case LdStructArrayElem:
    assert(inst.src(1)->isConst() && inst.src(1)->strVal()->isStatic());
    return PureLoad { AElemS { inst.src(0), inst.src(1)->strVal() } };

  // TODO(#5575265): replace this instruction with CheckTypeMem.
  case CheckTypePackedArrayElem:
  case IsPackedArrayElemNull:
    if (inst.src(1)->isConst() && inst.src(1)->intVal() >= 0) {
      return MayLoadStore {
        AElemI { inst.src(0), safe_cast<uint64_t>(inst.src(1)->intVal()) },
        AEmpty
      };
    }
    return MayLoadStore { AElemIAny, AEmpty };

  case InitPackedArrayLoop:
    {
      auto const extra = inst.extra<InitPackedArrayLoop>();
      return MayLoadStore {
        AStack {
          inst.src(1),
          extra->offset + static_cast<int32_t>(extra->size) - 1,
          static_cast<int32_t>(extra->size)
        },
        AElemIAny
      };
    }

  case NewStructArray:
    {
      // NewStructArray is reading elements from the stack, but writes to a
      // completely new array, so we can treat the store set as empty.
      auto const extra = inst.extra<NewStructArray>();
      return MayLoadStore {
        AStack {
          inst.src(0),
          extra->offset + static_cast<int32_t>(extra->numKeys) - 1,
          static_cast<int32_t>(extra->numKeys)
        },
        AEmpty
      };
    }

  //////////////////////////////////////////////////////////////////////
  // Member instructions

  /*
   * Various minstr opcodes that take a PtrToGen in src 0, which may or may not
   * point to a frame local or the evaluation stack.  These instructions can
   * all re-enter the VM and access arbitrary non-frame/stack locations, as
   * well.
   */
  case CGetElem:
  case ElemArray:
  case ElemArrayW:
  case ElemX:
  case EmptyElem:
  case IssetElem:
  case BindElem:
  case BindNewElem:
  case ElemDX:
  case ElemUX:
  case IncDecElem:
  case SetElem:
  case SetNewElemArray:
  case SetNewElem:
  case SetOpElem:
  case SetWithRefElem:
  case SetWithRefNewElem:
  case UnsetElem:
  case VGetElem:
    // Right now we generally can't limit any of these better than general
    // re-entry rules, since they can raise warnings and re-enter.
    assert(inst.src(0)->type() <= Type::PtrToGen);
    return MayLoadStore {
      AHeapAny | pointee(inst.src(0)) | reentry_extra(),
      ANonFrame | pointee(inst.src(0))
    };

  /*
   * These minstr opcodes either take a PtrToGen or an Obj as the base.  The
   * pointer may point at frame locals or the stack.  These instructions can
   * all re-enter the VM and access arbitrary non-frame/stack locations, as
   * well.
   */
  case CGetProp:
  case EmptyProp:
  case IssetProp:
  case PropX:
  case UnsetProp:
  case BindProp:
  case IncDecProp:
  case PropDX:
  case SetOpProp:
  case SetProp:
  case VGetProp:
    if (inst.src(0)->type() <= Type::PtrToGen) {
      return MayLoadStore {
        AHeapAny | pointee(inst.src(0)) | reentry_extra(),
        ANonFrame | pointee(inst.src(0))
      };
    }
    return MayLoadStore { AHeapAny | reentry_extra(), ANonFrame };

  /*
   * Collection accessors can read from their inner array buffer, but stores
   * COW and behave as if they only affect collection memory locations.  We
   * don't track those, so it's returning AEmpty for now.
   */
  case MapGet:
  case MapIsset:
  case MapSet:
  case PairIsset:
  case VectorDoCow:
  case VectorIsset:
    return MayLoadStore { AHeapAny, AEmpty /* Note */ };

  //////////////////////////////////////////////////////////////////////
  // Instructions that allocate new objects, so any effects they have on some
  // types of memory locations we track are isolated from anything else we care
  // about.

  case NewArray:
  case NewCol:
  case NewInstanceRaw:
  case NewLikeArray:
  case NewMIArray:
  case NewMixedArray:
  case NewMSArray:
  case NewVArray:
  case AllocPackedArray:
  case ConvBoolToArr:
  case ConvDblToStr:
  case ConvDblToArr:
  case ConvIntToArr:
  case ConvIntToStr:
  case ConvResToStr:
  case CreateSSWH:
  case Box:  // conditional allocation
    return IrrelevantEffects {};

  case AllocObj:  // AllocObj re-enters to call constructors.
    return MayLoadStore { AHeapAny | reentry_extra(), ANonFrame };

  //////////////////////////////////////////////////////////////////////
  // Instructions that explicitly manipulate the stack.

  case LdStack:
    return PureLoad {
      AStack { inst.src(0), inst.extra<LdStack>()->offset, 1 }
    };

  case StStk:
    return PureStore {
      AStack { inst.src(0), inst.extra<StStk>()->offset, 1 },
      inst.src(1)
    };

  case SpillFrame:
    return PureSpillFrame {
      AStack {
        inst.src(0),
        // SpillFrame's spOffset is to the bottom of where it will store the
        // ActRec, but AliasClass needs an offset to the highest cell it will
        // store.
        inst.extra<SpillFrame>()->spOffset + int32_t{kNumActRecCells} - 1,
        kNumActRecCells
      }
    };

  case GuardStk:
  case CheckStk:
    return MayLoadStore {
      AStack { inst.src(0), inst.extra<StackOffset>()->offset, 1 },
      AEmpty
    };
  case CufIterSpillFrame:
    return MayLoadStore { AEmpty, AStackAny };

  // The following may re-enter, and also deal with a stack slot.
  case CastStk:
  case CoerceStk:
    return MayLoadStore { ANonFrame | reentry_extra(), ANonFrame };

  case GuardRefs:
    // We're not bothering with being exact about where on the stack this
    // instruction can load, because it's always before anything else in a
    // region.
    return MayLoadStore { AStackAny, AEmpty };

  case LdARFuncPtr:
    // This instruction is essentially a PureLoad, but we don't handle non-TV's
    // in PureLoad so we have to treat it as MayLoadStore.  We also treat it as
    // loading an entire ActRec-sized part of the stack, although it only loads
    // the slot containing the Func.
    return MayLoadStore {
      AStack {
        inst.src(0),
        inst.extra<LdARFuncPtr>()->offset + int32_t{kNumActRecCells} - 1,
        int32_t{kNumActRecCells}
      },
      AEmpty
    };

  //////////////////////////////////////////////////////////////////////
  // Instructions that never do anything to memory

  case AssertStk:
  case HintStkInner:
  case AbsDbl:
  case AddDbl:
  case AddInt:
  case AddIntO:
  case AndInt:
  case AssertLoc:
  case AssertType:
  case DefFP:
  case DefSP:
  case EndGuards:
  case EqDbl:
  case EqInt:
  case GteInt:
  case GtInt:
  case HintLocInner:
  case Jmp:
  case JmpNZero:
  case JmpZero:
  case LdPropAddr:
  case LdStackAddr:
  case LteDbl:
  case LteInt:
  case LtInt:
  case GtDbl:
  case GteDbl:
  case LtDbl:
  case DivDbl:
  case MulDbl:
  case MulInt:
  case MulIntO:
  case NeqDbl:
  case NeqInt:
  case ReDefSP:
  case AdjustSP:
  case SubDbl:
  case SubInt:
  case SubIntO:
  case XorBool:
  case XorInt:
  case OrInt:
  case AssertNonNull:
  case CheckNonNull:
  case CheckNullptr:
  case Ceil:
  case Floor:
  case DefLabel:
  case DecRefNZ:
  case CheckInit:
  case Nop:
  case ClsNeq:
  case Mod:
  case TakeRef:
  case TakeStack:
  case Conjure:
  case DefMIStateBase:
  case Halt:
  case ConvBoolToInt:
  case ConvBoolToDbl:
  case DbgAssertType:
  case DefConst:
  case LdLocAddr:
  case Sqrt:
  case LdResumableArObj:
  case Shl:
  case Shr:
  case IsNType:
  case IsType:
  case Mov:
  case ConvClsToCctx:
  case ConvDblToBool:
  case ConvDblToInt:
  case IsScalarType:
  case LdMIStateAddr:
  case LdPairBase:
  case LdStaticLocCached:
  case CheckCtxThis:
  case CastCtxThis:
  case LdRDSAddr:
    return IrrelevantEffects {};

  //////////////////////////////////////////////////////////////////////
  // Instructions that technically do some things w/ memory, but not in any way
  // we currently care about.

  case CheckRefs:
  case ABCUnblock:
  case AFWHBlockOn:
  case LdClsCctx:
  case BeginCatch:
  case CheckSurpriseFlags:
  case CheckType:
  case FreeActRec:
  case IncRef:
  case IncRefCtx:
  case LdRetAddr:
  case RegisterLiveObj:
  case RetAdjustStack:
  case StClosureArg:
  case StClosureCtx:
  case StClosureFunc:
  case StContArKey:
  case StContArResume:
  case StContArState:
  case StContArValue:
  case StRetVal:
  case ZeroErrorLevel:
  case RestoreErrorLevel:
  case CheckCold:
  case CheckInitProps:
  case CheckInitSProps:
  case ContArIncIdx:
  case ContArIncKey:
  case ContArUpdateIdx:
  case ContValid:
  case ConcatIntStr:
  case ConcatStr3:
  case ConcatStr4:
  case ConcatStrInt:
  case ConcatStrStr:
  case CoerceStrToDbl:
  case CoerceStrToInt:
  case ConvStrToInt:
  case IncProfCounter:
  case IncStat:
  case IncStatGrouped:
  case CountBytecode:
  case ContPreNext:
  case ContStartedCheck:
  case ConvArrToBool:
  case ConvArrToDbl:
  case ConvArrToInt:
  case CoerceCellToBool:
  case ConvBoolToStr:
  case CountArray:
  case CountArrayFast:
  case StAsyncArResult:
  case StAsyncArResume:
  case StAsyncArSucceeded:
  case InstanceOf:
  case InstanceOfBitmask:
  case NInstanceOfBitmask:
  case InstanceOfIface:
  case InterfaceSupportsArr:
  case InterfaceSupportsDbl:
  case InterfaceSupportsInt:
  case InterfaceSupportsStr:
  case IsWaitHandle:
  case DbgAssertRefCount:
  case DbgAssertRetAddr:
  case NSame:
  case Same:
  case Gt:
  case Gte:
  case Eq:
  case Lt:
  case Lte:
  case Neq:
  case IncTransCounter:
  case LdBindAddr:
  case LdClsCtor:
  case LdAsyncArParentChain:
  case LdSSwitchDestFast:
  case LdSSwitchDestSlow:
  case RBTrace:
  case ConvCellToInt:
  case ConvIntToBool:
  case ConvIntToDbl:
  case ConvCellToDbl:
  case ConvObjToDbl:
  case ConvStrToArr:   // decrefs src, but src is a string
  case ConvStrToBool:
  case ConvStrToDbl:
  case DeleteUnwinderException:
  case DerefClsRDSHandle:
  case EagerSyncVMRegs:
  case ExtendsClass:
  case LdUnwinderValue:
  case GetCtxFwdCall:
  case LdCctx:
  case LdCtx:
  case LdClsName:
  case LdAFWHActRec:
  case LdClsCtx:
  case PrintBool:
  case PrintInt:
  case PrintStr:
  case LdContActRec:
  case LdContArKey:
  case LdContArValue:
  case LdContField:
  case LdContResumeAddr:
  case LdClsCachedSafe:
  case LdClsInitData:
  case UnwindCheckSideExit:
  case LdCns:
  case LdClsMethod:
  case LdClsMethodCacheCls:
  case LdClsMethodCacheFunc:
  case LdClsMethodFCacheFunc:
  case ProfilePackedArray:
  case ProfileStructArray:
  case LdFuncCachedSafe:
  case LdFuncNumParams:
  case LdGblAddr:
  case LdGblAddrDef:
  case LdObjClass:
  case LdObjInvoke:
  case LdStrLen:
  case StringIsset:
  case LdSwitchDblIndex:
  case LdSwitchStrIndex:
  case LdVectorBase:
  case LdWHResult:
  case LdWHState:
  case LookupClsRDSHandle:
  case AFWHPrepareChild:
  case CoerceCellToDbl:
  case CoerceCellToInt:
    return IrrelevantEffects {};

  // Some that touch memory we might care about later, but currently don't:
  case CheckStaticLocInit:
  case StaticLocInitCached:
  case ColAddElemC:
  case ColAddNewElemC:
  case ColIsEmpty:
  case ColIsNEmpty:
  case CheckBounds:
  case ConvCellToBool:
  case ConvObjToBool:
  case ConvObjToInt:
  case CountCollection:
  case LdVectorSize:
  case LdClsPropAddrOrNull:
  case LdClsPropAddrOrRaise:
  case VectorHasImmCopy:
  case CheckPackedArrayBounds:
  case LdColArray:
    return IrrelevantEffects {};

  //////////////////////////////////////////////////////////////////////
  // Instructions that can re-enter the VM and touch most heap things.  They
  // also may generally write to the eval stack below an offset (see
  // alias-class.h above AStack for more).

  case DecRefThis:
  case DecRef:
    return MayLoadStore { AHeapAny | reentry_extra(), ANonFrame };

  case DecRefStack:
    return MayLoadStore {
      AHeapAny | AStack { inst.src(0), inst.extra<DecRefStack>()->offset, 1 }
               | reentry_extra(),
      ANonFrame
    };

  case DecRefLoc:
    return MayLoadStore {
      AHeapAny | AFrame { inst.src(0), inst.extra<LocalId>()->locId }
               | reentry_extra(),
      ANonFrame
    };

  case LdArrFPushCuf:  // autoloads
  case LdArrFuncCtx:   // autoloads
  case LdObjMethod:    // can't autoload, but can decref $this right now
  case LdStrFPushCuf:  // autoload
    /*
     * Note that these instructions make stores to a pre-live actrec on the
     * eval stack.
     *
     * It is probably safe for these instructions to have may-load only from
     * the portion of the evaluation stack below the actrec they are
     * manipulating, but since there's always going to be either a Call or a
     * region exit following it it doesn't help us eliminate anything for now,
     * so we just pretend it can read anything on the stack.
     */
    return MayLoadStore { ANonFrame | reentry_extra(), ANonFrame };

  case BaseG:
  case Clone:
  case WarnNonObjProp:
  case RaiseArrayIndexNotice:
  case RaiseArrayKeyNotice:
  case RaiseError:
  case RaiseNotice:
  case RaiseUndefProp:
  case RaiseUninitLoc:
  case RaiseWarning:
  case ConvCellToStr:
  case ConvObjToStr:
  case ConcatCellCell:
  case Count:      // re-enters on CountableClass
  case CIterFree:  // decrefs context object in iter
  case MIterFree:
  case IterFree:
  case EqX:
  case GteX:
  case GtX:
  case LteX:
  case LtX:
  case NeqX:
  case DecodeCufIter:
  case ReleaseVVOrExit:  // can decref fields in an ExtraArgs structure
  case ConvCellToArr:  // decrefs src, may read obj props
  case ConvCellToObj:  // decrefs src
  case ConvObjToArr:   // decrefs src
  case CustomInstanceInit:
  case GenericIdx:
  case GetCtxFwdCallDyn: // autoload in StaticMethodCache
  case InitProps:
  case InitSProps:
  case OODeclExists:
  case LdCls:          // autoload
  case LdClsCached:    // autoload
  case LdFunc:         // autoload
  case LdFuncCached:   // autoload
  case LdFuncCachedU:  // autoload
  case LdSwitchObjIndex:  // decrefs arg
  case LookupClsCns:      // autoload
  case LookupClsMethod:   // autoload
  case LookupClsMethodCache:  // autoload
  case LookupClsMethodFCache: // autoload
  case LookupCns:
  case LookupCnsE:
  case LookupCnsU:
  case StringGet:   // raise_warning
  case ArrayAdd:    // decrefs source
  case AKExists:    // re-enters for warnings on kVPackedKind, etc
  case AddElemIntKey:  // decrefs value
  case AddElemStrKey:  // decrefs value
  case AddNewElem:     // decrefs value
  case ArrayIdx:       // kVPackedKind warnings
  case ArrayGet:       // kVPackedKind warnings
  case ArrayIsset:     // kVPackedKind warnings
  case ArraySet:       // kVPackedKind warnings
  case ArraySetRef:    // kVPackedKind warnings
  case GetMemoKey:  // re-enters to call getInstanceKey() in some cases
    return MayLoadStore { AHeapAny | reentry_extra(), ANonFrame };

  //////////////////////////////////////////////////////////////////////
  // The following instructions are used for debugging memory optimizations, so
  // this analyzer should pretend they don't exist.

  case DbgTrashStk:
  case DbgTrashFrame:
    return IrrelevantEffects {};

  //////////////////////////////////////////////////////////////////////

  }

  not_reached();
}

//////////////////////////////////////////////////////////////////////

bool check_effects(const IRInstruction& inst, MemEffects me) {
  auto check_fp = [&] (SSATmp* fp) {
    always_assert_flog(
      fp->type() <= Type::FramePtr,
      "Non frame pointer in memory effects:\n  inst: {}\n  effects: {}",
      inst.toString(),
      show(me)
    );
  };

  auto check_obj = [&] (SSATmp* obj) {
    always_assert_flog(
      // Maybe we should actually just give up on this (or check it /before/
      // doing canonicalize?).
      obj->type() <= Type::Obj,
      "Non obj pointer in memory effects:\n  inst: {}\n  effects: {}",
      inst.toString(),
      show(me)
    );
  };

  auto check = [&] (AliasClass a) {
    if (auto const fr = a.frame()) {
      check_fp(fr->fp);
      return;
    }
    if (auto const pr = a.prop()) {
      check_obj(pr->obj);
    }
  };

  // In debug let's do some type checking in case people move instruction
  // argument numbers.
  match<void>(
    me,
    [&] (MayLoadStore m)    { check(m.loads); check(m.stores); },
    [&] (PureLoad m)        { check(m.src); },
    [&] (PureStore m)       { check(m.dst); },
    [&] (PureStoreNT m)     { check(m.dst); },
    [&] (PureSpillFrame m)  { check(m.dst); },
    [&] (IterEffects m)     { check_fp(m.fp); check(m.killed); },
    [&] (IterEffects2 m)    { check_fp(m.fp); check(m.killed); },
    [&] (KillFrameLocals m) { check_fp(m.fp); },
    [&] (ExitEffects m)     { check(m.live); check(m.kill); },
    [&] (IrrelevantEffects) {},
    [&] (UnknownEffects)    {},
    [&] (InterpOneEffects m){ check(m.killed); },
    [&] (CallEffects m)     { check(m.killed); },
    [&] (ReturnEffects m)   { check(m.killed); }
  );

  return true;
}

//////////////////////////////////////////////////////////////////////

}

MemEffects memory_effects(const IRInstruction& inst) {
  auto const ret = memory_effects_impl(inst);
  assert(check_effects(inst, ret));
  return ret;
}

//////////////////////////////////////////////////////////////////////

MemEffects canonicalize(MemEffects me) {
  using R = MemEffects;
  return match<R>(
    me,
    [&] (MayLoadStore l) -> R {
      return MayLoadStore { canonicalize(l.loads), canonicalize(l.stores) };
    },
    [&] (PureLoad l) -> R {
      return PureLoad { canonicalize(l.src) };
    },
    [&] (PureStore l) -> R {
      return PureStore { canonicalize(l.dst), l.value };
    },
    [&] (PureStoreNT l) -> R {
      return PureStoreNT { canonicalize(l.dst), l.value };
    },
    [&] (PureSpillFrame l) -> R {
      return PureSpillFrame { canonicalize(l.dst) };
    },
    [&] (ExitEffects l) -> R {
      return ExitEffects { canonicalize(l.live), canonicalize(l.kill) };
    },
    [&] (CallEffects l) -> R {
      return CallEffects { l.destroys_locals, canonicalize(l.killed) };
    },
    [&] (ReturnEffects l) -> R {
      return ReturnEffects { canonicalize(l.killed) };
    },
    [&] (IterEffects l) -> R {
      return IterEffects { l.fp, l.id, canonicalize(l.killed) };
    },
    [&] (IterEffects2 l) -> R {
      return IterEffects2 { l.fp, l.id1, l.id2, canonicalize(l.killed) };
    },
    [&] (InterpOneEffects l) -> R {
      return InterpOneEffects { canonicalize(l.killed) };
    },
    [&] (KillFrameLocals l)   -> R { return l; },
    [&] (IrrelevantEffects l) -> R { return l; },
    [&] (UnknownEffects l)    -> R { return l; }
  );
}

//////////////////////////////////////////////////////////////////////

std::string show(MemEffects effects) {
  using folly::sformat;
  return match<std::string>(
    effects,
    [&] (MayLoadStore m) {
      return sformat("mls({} ; {})", show(m.loads), show(m.stores));
    },
    [&] (ExitEffects m) {
      return sformat("exit({} ; {})", show(m.live), show(m.kill));
    },
    [&] (CallEffects m) {
      return sformat("call({})", show(m.killed));
    },
    [&] (InterpOneEffects m) {
      return sformat("interp({})", show(m.killed));
    },
    [&] (PureLoad m)        { return sformat("ld({})", show(m.src)); },
    [&] (PureStore m)       { return sformat("st({})", show(m.dst)); },
    [&] (PureStoreNT m)     { return sformat("stNT({})", show(m.dst)); },
    [&] (PureSpillFrame m)  { return sformat("stFrame({})", show(m.dst)); },
    [&] (ReturnEffects m)   { return sformat("return({})", show(m.killed)); },
    [&] (IterEffects)       { return "IterEffects"; },
    [&] (IterEffects2)      { return "IterEffects2"; },
    [&] (KillFrameLocals)   { return "KillFrameLocals"; },
    [&] (IrrelevantEffects) { return "IrrelevantEffects"; },
    [&] (UnknownEffects)    { return "UnknownEffects"; }
  );
}

//////////////////////////////////////////////////////////////////////

}}
