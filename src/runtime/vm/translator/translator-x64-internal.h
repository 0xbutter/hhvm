/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#ifndef incl_TRANSLATOR_X64_INTERNAL_H_
#define incl_TRANSLATOR_X64_INTERNAL_H_

#include <boost/optional.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#include <runtime/vm/translator/abi-x64.h>

/* 
 * Please don't include this unless your file implements methods of
 * TranslatorX64; you won't like it. It pollutes the namespace, makes
 * "KindOfString" #error, makes your TRACEMOD tx64, and tortures a kitten.
 */

using namespace HPHP::VM::Transl::reg;
using namespace HPHP::Util;
using namespace HPHP::Trace;
using std::max;

namespace HPHP {
namespace VM {
namespace Transl {

static const Trace::Module TRACEMOD = Trace::tx64;
static const DataType BitwiseKindOfString = KindOfString;

#define KindOfString \
#error You probably do not mean to use KindOfString in this file.

// RAII aids to machine code.

// Put FreezeRegs in a scope around any emit calls where the caller needs
// to be sure the callee will not modify the state of the register
// map.  (Arises in situations with conditional code often.)
struct FreezeRegs : private boost::noncopyable {
  explicit FreezeRegs(RegAlloc& regs) : m_regs(regs) { m_regs.freeze(); }
  ~FreezeRegs() { m_regs.defrost(); }

private:
  RegAlloc& m_regs;
};

class RedirectSpillFill : boost::noncopyable {
  X64Assembler* const m_oldSpf;
public:
  explicit RedirectSpillFill(X64Assembler* newCode)
    : m_oldSpf(tx64->m_spillFillCode)
  {
    tx64->m_spillFillCode = newCode;
  }
  ~RedirectSpillFill() {
    tx64->m_spillFillCode = m_oldSpf;
  }
};

// DiamondGuard is a scoped way to protect register allocator state around
// control flow. When we enter some optional code that may affect the state
// of the register file, we copy the register file's state, and redirect any
// spills and fills to the branch's body.
//
// When we're ready to rejoin the main control flow, we bring the registers
// back into the state they were in, and restore the old spill/fill
// destinations.
class DiamondGuard : boost::noncopyable {
  RedirectSpillFill m_spfChanger;
public:
  explicit DiamondGuard(X64Assembler& a) : m_spfChanger(&a) {
    tx64->m_savedRegMaps.push(
      TranslatorX64::SavedRegState(this, tx64->m_regMap));
  }
  ~DiamondGuard() {
    ASSERT(!tx64->m_savedRegMaps.empty());
    ASSERT(tx64->m_savedRegMaps.top().saver == this);

    // Bring the register state back to its state in the main body.
    //
    // Note: it's important that tx64->m_regMap is the branch
    // RegAlloc during this.  See RegAlloc::reconcile.
    tx64->m_savedRegMaps.top().savedState.reconcile(tx64->m_regMap);
    tx64->m_regMap = tx64->m_savedRegMaps.top().savedState;
    tx64->m_savedRegMaps.pop();
  }
};

// Helper for use with UnlikelyIfBlock when you have a complex else
// branch that needs to make changes to the register file.
//
// For an example usage see UnlikelyIfBlock.
class DiamondReturn : boost::noncopyable {
  X64Assembler* m_branchA;
  X64Assembler* m_mainA;
  TCA m_branchJmp;
  TCA m_finishBranchFrontier;

private:
  template<int> friend class UnlikelyIfBlock;

  void initBranch(X64Assembler* branchA, X64Assembler* mainA) {
    /*
     * DiamondReturn must be used with branches going to different
     * code regions.
     */
    ASSERT(branchA != mainA);

    m_branchA = branchA;
    m_mainA = mainA;
    tx64->m_savedRegMaps.push(
      TranslatorX64::SavedRegState(this, tx64->m_regMap));
  }

  void finishBranch(TCA jmp, TCA frontier) {
    m_branchJmp = jmp;
    m_finishBranchFrontier = frontier;

    // If there's some reason to do something other than this we have
    // to change the way this class works.
    const int UNUSED kJumpSize = 5;
    ASSERT(m_finishBranchFrontier == m_branchJmp + kJumpSize);

    // We're done with the branch, so save the branch's state and
    // switch back to the main line's state.
    swapRegMaps();
  }

private:
  bool finishedBranch() const { return m_branchJmp != 0; }

  void swapRegMaps() {
    ASSERT(!tx64->m_savedRegMaps.empty());
    ASSERT(tx64->m_savedRegMaps.top().saver == this);
    std::swap(tx64->m_savedRegMaps.top().savedState, tx64->m_regMap);
  }

  void emitReconciliation() {
    ASSERT(!tx64->m_savedRegMaps.empty());
    ASSERT(tx64->m_savedRegMaps.top().saver == this);

    RedirectSpillFill spfRedir(m_branchA);

    if (finishedBranch()) {
      // We need tx64->m_regMap to point at the branch during
      // reconciliation.  See RegAlloc::reconcile().
      swapRegMaps();
    }
    RegAlloc& branchState = tx64->m_regMap;
    RegAlloc& currentState = tx64->m_savedRegMaps.top().savedState;
    currentState.reconcile(branchState);
    tx64->m_regMap = currentState;
    tx64->m_savedRegMaps.pop();
  }

public:
  explicit DiamondReturn()
    : m_branchA(0)
    , m_branchJmp(0)
  {}

  ~DiamondReturn() {
    ASSERT(m_branchA &&
      "DiamondReturn was created without being passed to UnlikelyIfBlock");

    if (!finishedBranch()) {
      /*
       * In this case, we're reconciling the branch even though it
       * isn't really finished (no one ever called finishBranch()), so
       * we just need to emit spills/fills now and not be as clever as
       * below.  See UnlikelyIfBlock::reconcileEarly.
       */
      emitReconciliation();
      return;
    }

    const TCA currentBranchFrontier = m_branchA->code.frontier;
    const bool branchFrontierMoved =
      currentBranchFrontier != m_finishBranchFrontier;

    /*
     * If the branch frontier hasn't moved since the branch was
     * finished, we don't need the jmp that was already emitted
     * anymore, so rewind so we can potentially overwrite it with
     * spills/fills.
     */
    if (!branchFrontierMoved) {
      m_branchA->code.frontier = m_branchJmp;
    }

    // Send out reconciliation code to the branch area.  We want to
    // bring the state of the branch's register file into sync with
    // the main-line.
    const TCA spfStart = m_branchA->code.frontier;
    emitReconciliation();
    const bool hadAnySpf = spfStart != m_branchA->code.frontier;

    if (branchFrontierMoved) {
      /*
       * In this case, more than one DiamondReturn is being used and
       * there are multiple unlikely branches.
       *
       * If there was no reconciliation code it's not big deal, we'll
       * just patch the existing jmp to go to the return in m_mainA.
       * But if we needed reconciliation code, we'll instead want to
       * patch it to jump there.
       */
      if (hadAnySpf) {
        m_branchA->patchJmp(m_branchJmp, spfStart);
        m_branchA->jmp(m_mainA->code.frontier);
      } else {
        m_branchA->patchJmp(m_branchJmp, m_mainA->code.frontier);
      }
    } else {
      ASSERT(spfStart == m_branchJmp);
      m_branchA->jmp(m_mainA->code.frontier);
    }
  }
};
// UnlikelyIfBlock:
//
//  Branch to distant code (that we presumably don't expect to
//  take). This helps keep hot paths compact.
//
//  A common pattern using this involves patching the jump in astubs
//  to jump past the normal control flow in a (as in the following
//  example).  Do this using DiamondReturn so the register allocator
//  state will be properly maintained.  (Spills/fills to keep the
//  states in sync will be emitted on the unlikely path.)
//
// Example:
//
//  {
//    PhysReg inputParam = i.getReg(i.inputs[0]->location);
//    a.   test_reg_reg(inputParam, inputParam);
//    DiamondReturn retFromStubs;
//    {
//      UnlikelyIfBlock<CC_Z> ifNotRax(a, astubs, &retFromStubs);
//      EMIT_CALL(a, TCA(launch_nuclear_missiles));
//    }
//    // The inputParam was non-zero, here is the likely branch:
//    m_regMap.allocOutputRegs(i);
//    emitMovRegReg(inputParam, m_regMap.getReg(i.outLocal->location));
//    // ~DiamondReturn patches the jump, and reconciles the branch
//    // with the main line.  (In this case it will fill the outLocal
//    // register since the main line thinks it is dirty.)
//  }
//  // The two cases are joined here.  We can do logic that was
//  // independent of whether the branch was taken, if necessary.
//  emitMovRegReg(i.outLocal, m_regMap.getReg(i.outStack->location));
//
// Note: it is ok to nest UnlikelyIfBlocks, as long as their
// corresponding DiamondReturns are correctly destroyed in reverse
// order.  But also note that this can lead to more jumps on the
// unlikely branch (see ~DiamondReturn).
template <int Jcc>
struct UnlikelyIfBlock {
  X64Assembler& m_likely;
  X64Assembler& m_unlikely;
  TCA m_likelyPostBranch;

  DiamondReturn* m_returnDiamond;
  bool m_externalDiamond;
  boost::optional<FreezeRegs> m_ice;

  explicit UnlikelyIfBlock(X64Assembler& likely,
                           X64Assembler& unlikely,
                           DiamondReturn* returnDiamond = 0)
    : m_likely(likely)
    , m_unlikely(unlikely)
    , m_returnDiamond(returnDiamond ? returnDiamond : new DiamondReturn())
    , m_externalDiamond(!!returnDiamond)
  {
    m_likely.jcc(Jcc, m_unlikely.code.frontier);
    m_likelyPostBranch = m_likely.code.frontier;
    m_returnDiamond->initBranch(&unlikely, &likely);
    tx64->m_spillFillCode = &unlikely;
  }

  ~UnlikelyIfBlock() {
    TCA jmpAddr = m_unlikely.code.frontier;
    m_unlikely.jmp(m_likelyPostBranch);
    if (m_returnDiamond) {
      m_returnDiamond->finishBranch(jmpAddr, m_unlikely.code.frontier);
      if (!m_externalDiamond) {
        delete m_returnDiamond;
      }
    }
    tx64->m_spillFillCode = &m_likely;
  }

  /*
   * Force early reconciliation between the branch and main line.
   * Using this has some tricky cases, part of which is that you can't
   * allocate registers anymore in the branch if you do this (so we'll
   * freeze until ~UnlikelyIfBlock).
   *
   * It's also almost certainly error if we have m_externalDiamond, so
   * for now that's an assert.
   */
  void reconcileEarly() {
    ASSERT(!m_externalDiamond);
    delete m_returnDiamond;
    m_returnDiamond = 0;
    m_ice = boost::in_place<FreezeRegs>(boost::ref(tx64->m_regMap));
  }
};

// A CondBlock is an RAII structure for emitting conditional code. It
// compares the source register at fieldOffset with fieldValue, and
// conditionally branches over the enclosing block of assembly on the
// passed-in condition-code.
//
//  E.g.:
//    {
//      RefCountedOnly ifRefCounted(a, rdi, 0);
//      emitIncRef(rdi);
//    }
//
// will only execute emitIncRef if we find at runtime that rdi points at
// a ref-counted cell.
//
// It's ok to do reconcilable register operations in the body.
template<int FieldOffset, int FieldValue, int Jcc>
struct CondBlock {
  X64Assembler& m_a;
  int m_off;
  TCA m_jcc8;
  DiamondGuard* m_dg;

  CondBlock(X64Assembler& a, PhysReg reg, int offset = 0)
      : m_a(a), m_off(offset), m_dg(new DiamondGuard(a)) {
    int typeDisp = m_off + FieldOffset;
    a.   cmp_imm32_disp_reg32(FieldValue, typeDisp, reg);
    m_jcc8 = a.code.frontier;
    a.   jcc8(Jcc, m_jcc8);
    // ...
  }

  ~CondBlock() {
    delete m_dg;
    m_a.patchJcc8(m_jcc8, m_a.code.frontier);
  }
};

// IfRefCounted --
//   Emits if (IS_REFCOUNTED_TYPE()) { ... }
typedef CondBlock <TVOFF(m_type),
                   KindOfRefCountThreshold,
                   CC_LE> IfRefCounted;

typedef CondBlock <TVOFF(m_type),
                   KindOfRef,
                   CC_NZ> IfVariant;

typedef CondBlock <TVOFF(m_type),
                   KindOfUninit,
                   CC_Z> UnlessUninit;

/*
 * locToRegDisp --
 *
 * Helper code for stack frames. The struct is a "template" in the
 * non-C++ sense: we don't build source-level stack frames in C++
 * for the most part, but its offsets tell us where to find fields
 * in assembly.
 *
 * If we were physically pushing stack frames, we would push them
 * in reverse order to what you see here.
 */
static void
locToRegDisp(const Location& l, PhysReg *outbase, int *outdisp,
             const Func* f = NULL) {
  assert_not_implemented((l.space == Location::Stack ||
                          l.space == Location::Local ||
                          l.space == Location::Iter));
  *outdisp = cellsToBytes(Translator::locPhysicalOffset(l, f));
  *outbase = l.space == Location::Stack ? rVmSp : rVmFp;
}

// Common code emission patterns.

// emitStoreImm --
//   Try to use a nice encoding for the size and value.
static void
emitStoreImm(X64Assembler& a, uint64_t imm, PhysReg r, int off,
             int size = sz::qword, RegAlloc* regAlloc = NULL) {
  if (size == sz::qword) {
    PhysReg immReg = regAlloc ? regAlloc->getImmReg(imm) : InvalidReg;
    if (immReg == InvalidReg) {
      emitImmReg(a, imm, rScratch);
      immReg = rScratch;
    }
    a.   store_reg64_disp_reg64(immReg, off, r);
  } else if (size == sz::dword) {
    a.   store_imm32_disp_reg(imm, off, r);
  } else {
    not_implemented();
  }
}

// vstackOffset --
// emitVStackStore --
//
//   Store to the virtual stack at a normalized instruction boundary.
//   Since rVmSp points to the stack at entry to the current BB, we need to
//   adjust stack references relative to it.
//
//   For all parameters, offsets and sizes are in bytes. Sizes are expected
//   to be a hardware size: 1, 2, 4, or 8 bytes.
static inline int
vstackOffset(const NormalizedInstruction& ni, COff off) {
  return off - cellsToBytes(ni.stackOff);
}

static inline void
emitVStackStoreImm(X64Assembler &a, const NormalizedInstruction &ni,
                   uint64_t imm, int off, int size = sz::qword,
                   RegAlloc *regAlloc = NULL) {
  int hwOff = vstackOffset(ni, off);
  emitStoreImm(a, imm, rVmSp, hwOff, size, regAlloc);
}

static inline void
emitVStackStore(X64Assembler &a, const NormalizedInstruction &ni,
                PhysReg src, int off, int size = sz::qword) {
  int hwOff = vstackOffset(ni, off);
  if (size == sz::qword) {
    a.    store_reg64_disp_reg64(src, hwOff, rVmSp);
  } else if (size == sz::dword) {
    a.    store_reg32_disp_reg64(src, hwOff, rVmSp);
  } else {
    not_implemented();
  }
}

static inline const StringData*
local_name(const Location& l) {
  ASSERT(l.isLocal());
  const StringData* ret = curFunc()->localNames()[l.offset];
  ASSERT(ret->isStatic());
  return ret;
}

// emitDispDeref --
// emitDeref --
// emitStoreTypedValue --
// emitStoreUninitNull --
// emitStoreNull --
//
//   Helpers for common cell operations.
//
//   Dereference the var in the cell whose address lives in src into
//   dest.
static void
emitDispDeref(X64Assembler &a, PhysReg src, int disp,
              PhysReg dest) {
  a.    load_reg64_disp_reg64(src, disp + TVOFF(m_data), dest);
}

static void
emitDeref(X64Assembler &a, PhysReg src, PhysReg dest) {
  emitDispDeref(a, src, 0, dest);
}

static void
emitDerefIfVariant(X64Assembler &a, PhysReg reg) {
  if (RuntimeOption::EvalJitCmovVarDeref) {
    a.cmp_imm32_disp_reg32(KindOfRef, TVOFF(m_type), reg);
    a.cload_reg64_disp_reg64(CC_Z, reg, TVOFF(m_data), reg);
  } else {
    IfVariant ifVar(a, reg);
    emitDeref(a, reg, reg);
  }
}

// NB: leaves count field unmodified. Does not store to m_data if type
// is a null type.
static inline void
emitStoreTypedValue(X64Assembler& a, DataType type, PhysReg val,
                    int disp, PhysReg dest, bool writeType = true) {
  if (writeType) {
    a.  store_imm32_disp_reg(type, disp + TVOFF(m_type), dest);
  }
  if (!IS_NULL_TYPE(type)) {
    ASSERT(val != reg::noreg);
    a.  store_reg64_disp_reg64(val, disp + TVOFF(m_data), dest);
  }
}

static inline void
emitStoreInvalid(X64Assembler& a, int disp, PhysReg dest) {
  a.    store_imm64_disp_reg64(0xfacefacefacefaceULL, disp + TVOFF(m_data),
                               dest);
  a.    store_imm32_disp_reg(0xfacefaceU, disp + TVOFF(_count), dest);
  a.    store_imm32_disp_reg(KindOfInvalid, disp + TVOFF(m_type), dest);
}

static inline void
emitStoreUninitNull(X64Assembler& a,
                    int disp,
                    PhysReg dest) {
  // OK to leave garbage in m_data, _count.
  a.    store_imm32_disp_reg(KindOfUninit, disp + TVOFF(m_type), dest);
}

static inline void
emitStoreNull(X64Assembler& a,
              int disp,
              PhysReg dest) {
  a.    store_imm32_disp_reg(KindOfNull, disp + TVOFF(m_type), dest);
  // It's ok to leave garbage in m_data, _count for KindOfNull.
}

static inline void
emitStoreNull(X64Assembler& a, const Location& where) {
  PhysReg base;
  int disp;
  locToRegDisp(where, &base, &disp);
  emitStoreNull(a, disp, base);
}

/*
 * The 'zero' argument can be noreg, rFlag, or a normal register
 * name. If it's noreg, a 0 immediate will be stored to _count. If
 * it's rFlag, nothing will be stored to _count. If it's a normal
 * register name, the contents of that register (hopefully set to zero
 * by the caller) are stored to _count.
 */
static inline void
emitCopyTo(X64Assembler& a,
           PhysReg src,
           int srcOff,
           PhysReg dest,
           int destOff,
           PhysReg scratch) {
  ASSERT(src != scratch);
  // This is roughly how gcc compiles this.
  a.    load_reg64_disp_reg64(src, srcOff + TVOFF(m_data), scratch);
  // Blow off _count.
  a.    store_reg64_disp_reg64(scratch, destOff + TVOFF(m_data), dest);
  a.    load_reg64_disp_reg32(src, srcOff + TVOFF(m_type), scratch);
  a.    store_reg32_disp_reg64(scratch, destOff + TVOFF(m_type), dest);
}

// ArgManager -- support for passing VM-level data to helper functions.
class ArgManager {
  typedef HPHP::VM::Transl::X64Assembler& A;
public:
  ArgManager(TranslatorX64 &tx64, A& a) : m_tx64(tx64), m_a(a) { }

  void addImm(uint64_t imm) {
    TRACE(6, "ArgManager: push arg %zd imm:%lu\n",
          m_args.size(), imm);
    m_args.push_back(ArgContent(ArgContent::ArgImm, InvalidReg, imm));
  }

  void addLoc(const Location &loc) {
    TRACE(6, "ArgManager: push arg %zd loc:(%s, %lld)\n",
          m_args.size(), loc.spaceName(), loc.offset);
    m_args.push_back(ArgContent(ArgContent::ArgLoc, loc));
  }

  void addDeref(const Location &loc) {
    TRACE(6, "ArgManager: push arg %zd deref:(%s, %lld)\n",
          m_args.size(), loc.spaceName(), loc.offset);
    m_args.push_back(ArgContent(ArgContent::ArgDeref, loc));
  }

  void addReg(PhysReg reg) {
    TRACE(6, "ArgManager: push arg %zd reg:r%d\n",
          m_args.size(), reg);
    m_args.push_back(ArgContent(ArgContent::ArgReg, reg, 0));
  }

  void addRegPlus(PhysReg reg, int32_t off) {
    TRACE(6, "ArgManager: push arg %zd regplus:r%d+%d\n",
          m_args.size(), reg, off);
    m_args.push_back(ArgContent(ArgContent::ArgRegPlus, reg, off));
  }

  void addLocAddr(const Location &loc) {
    TRACE(6, "ArgManager: push arg %zd addr:(%s, %lld)\n",
          m_args.size(), loc.spaceName(), loc.offset);
    ASSERT(!loc.isLiteral());
    m_args.push_back(ArgContent(ArgContent::ArgLocAddr, loc));
  }

  void addLiteral(const Location &loc, A &a, RegAlloc &regMap,
                  PhysReg rMis) {
    ASSERT(loc.isLiteral());
    if (loc.space == Location::Litstr) {
      StringData* sd = curUnit()->lookupLitstrId(loc.offset);
      TRACE(6, "ArgManager: push arg %zd (%s, %lld) \"%s\" --> r%d+%zu\n",
            m_args.size(), loc.spaceName(), loc.offset, sd->data(), rMis,
            offsetof(TranslatorX64::MInstrState, tvLiteral));
      a.  store_imm64_disp_reg64((uintptr_t)sd,
                                 offsetof(TranslatorX64::MInstrState, tvLiteral)
                                 + TVOFF(m_data), rMis);
      a.  store_imm32_disp_reg(BitwiseKindOfString,
                               offsetof(TranslatorX64::MInstrState, tvLiteral)
                               + TVOFF(m_type), rMis);
    } else {
      ASSERT(loc.space == Location::Litint);
      TRACE(6, "ArgManager: push arg %zd (%s, %lld) --> r%d+%zu\n",
            m_args.size(), loc.spaceName(), loc.offset, rMis,
            offsetof(TranslatorX64::MInstrState, tvLiteral));
      a.  store_imm64_disp_reg64(loc.offset,
                                 offsetof(TranslatorX64::MInstrState, tvLiteral)
                                 + TVOFF(m_data), rMis);
      a.  store_imm32_disp_reg(KindOfInt64,
                               offsetof(TranslatorX64::MInstrState, tvLiteral)
                               + TVOFF(m_type), rMis);
    }
    m_args.push_back(ArgContent(ArgContent::ArgRegPlus, rMis,
                                offsetof(TranslatorX64::MInstrState,
                                         tvLiteral)));
  }

  void emitArguments() {
    size_t n = m_args.size();
    ASSERT((int)n <= kNumRegisterArgs);
    cleanLocs();
    std::map<PhysReg, size_t> used;
    std::vector<PhysReg> actual(n, InvalidReg);
    computeUsed(used, actual);
    shuffleRegisters(used, actual);
    emitValues(actual);
  }

private:
  struct ArgContent {
    enum ArgKind {
      ArgImm, ArgLoc, ArgDeref, ArgReg, ArgRegPlus, ArgLocAddr
    } m_kind;
    PhysReg m_reg;
    const Location *m_loc;
    uint64_t m_imm;

    ArgContent(ArgKind kind, PhysReg reg, uint64_t imm) :
      m_kind(kind), m_reg(reg), m_loc(NULL), m_imm(imm) { }
    ArgContent(ArgKind kind, const Location &loc) :
      m_kind(kind), m_reg(InvalidReg), m_loc(&loc), m_imm(0) { }
  };

  TranslatorX64& m_tx64;
  A& m_a;
  std::vector<ArgContent> m_args;

  ArgManager(); // Don't build without reference to translator

  void cleanLocs();
  void computeUsed(std::map<PhysReg, size_t> &used,
                   std::vector<PhysReg> &actual);
  void shuffleRegisters(std::map<PhysReg, size_t> &used,
                     std::vector<PhysReg> &actual);
  void emitValues(std::vector<PhysReg> &actual);
};

// Some macros to make writing calls palatable. You have to "type" the
// arguments
#define IMM(i)       _am.addImm(i)
#define V(loc)       _am.addLoc(loc)
#define DEREF(loc)   _am.addDeref(loc)
#define R(r)         _am.addReg(r)
#define RPLUS(r,off) _am.addRegPlus(r, off)
#define A(loc)       _am.addLocAddr(loc)
#define IE(cond, argIf, argElse) \
  ((cond) ? (argIf) : (argElse))

#define PREP_CTX(ctxFixed, pr)                                         \
  Class* ctx = NULL;                                                   \
  LazyScratchReg rCtx(m_regMap);                                       \
  if (ctxFixed) {                                                      \
    ASSERT(isContextFixed());                                          \
    ctx = arGetContextClass(curFrame());                               \
  } else {                                                             \
    rCtx.alloc(pr);                                                    \
    a.  load_reg64_disp_reg64(rsp, offsetof(MInstrState, ctx), *rCtx); \
  }
#define CTX(ctxFixed)     \
  IE((ctxFixed),          \
     IMM(uintptr_t(ctx)), \
     R(*rCtx))

#define EMIT_CALL_PROLOGUE(a) do { \
  SpaceRecorder sr("_HCallInclusive", a);  \
  ArgManager _am(*this, a);       \
  prepareCallSaveRegs();

#define EMIT_CALL_EPILOGUE(a, dest) \
  _am.emitArguments();           \
  { \
    SpaceRecorder sr("_HCallExclusive", a); \
    emitCall(a, (TCA)(dest), true);         \
  } \
} while(0)

#define EMIT_CALL(a, dest, ...) \
  EMIT_CALL_PROLOGUE(a)         \
  __VA_ARGS__ ;                 \
  EMIT_CALL_EPILOGUE(a, dest)

#define EMIT_RCALL(a, ni, dest, ...) \
  EMIT_CALL(a, dest, __VA_ARGS__);   \
  recordReentrantCall(a, ni);

// typeReentersOnRelease --
//   Returns whether the release helper for a given type can
//   reenter.
static bool typeReentersOnRelease(DataType type) {
  return IS_REFCOUNTED_TYPE(type) && type != BitwiseKindOfString;
}

// supportedPlan --
// nativePlan --
// simplePlan --
//   Some helpers for analyze* methods.
static inline TXFlags
plan(bool cond, TXFlags successFlags, TXFlags fallbackFlags=Interp) {
  return cond ? successFlags : fallbackFlags;
}

static inline TXFlags simplePlan(bool cond) { return plan(cond, Simple); }
static inline TXFlags simpleOrSupportedPlan(bool cond) {
  return plan(cond, Simple, Supported);
}
static inline TXFlags supportedPlan(bool cond) { return plan(cond, Supported); }
static inline TXFlags nativePlan(bool cond) { return plan(cond, Native); }

static inline TXFlags planHingesOnRefcounting(DataType type) {
  return !IS_REFCOUNTED_TYPE(type) ? Native :
         !typeReentersOnRelease(type) ? Simple :
         Supported;
}

static const char* getContextName() {
  ASSERT(isContextFixed());
  Class* ctx = arGetContextClass(curFrame());
  return ctx ? ctx->name()->data() : ":anonymous:";
}

}}}

#endif

