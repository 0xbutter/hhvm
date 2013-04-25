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

#ifndef incl_HPHP_VM_IRFACTORY_H_
#define incl_HPHP_VM_IRFACTORY_H_

#include <type_traits>
#include <vector>

#include "folly/ScopeGuard.h"
#include "util/arena.h"
#include "runtime/vm/translator/hopt/ir.h"
#include "runtime/vm/translator/hopt/cse.h"

namespace HPHP { namespace VM { namespace JIT {

//////////////////////////////////////////////////////////////////////

/*
 * ReturnValue makeInstruction(Lambda, Args...) --
 *
 *   Create an IRInstruction on the stack using Args, and call Lambda
 *   with a pointer to it, returning the result.
 *
 *   Normally IRInstruction creation should go through either
 *   IRFactory::gen or TraceBuilder::gen.  This utility is used to
 *   implement those.  The lambda must not escape the IRInstruction*.
 */

namespace detail {

template<class Ret, class Func>
struct InstructionBuilder {
  explicit InstructionBuilder(const Func& func) : func(func) {}

  /*
   * Create an IRInstruction, and then recursively chew on the Args
   * list to populate its fields.
   *
   * The IRInstruction is stack allocated, and should not escape the
   * lambda, so we fill it with 0xc0 in debug builds after we're done.
   */
  template<class... Args>
  Ret go(Opcode op, Args&&... args) {
    std::aligned_storage<
      sizeof(IRInstruction)
    >::type buffer;
    void* const vpBuffer = &buffer;
    SCOPE_EXIT { if (debug) memset(&buffer, 0xc0, sizeof buffer); };

    new (vpBuffer) IRInstruction(op);
    auto const inst = static_cast<IRInstruction*>(vpBuffer);

    SCOPE_EXIT { inst->clearExtra(); };
    return go2(inst, std::forward<Args>(args)...);
  }

private:
  // Main loop: call setter() on the head of the list and repeat,
  // until there's no overload for the Head type; then it will fall
  // through to the base case.
  template<class Head, class... Tail>
  Ret go2(IRInstruction* inst, const Head& x, Tail&&... xs) {
    setter(inst, x);
    return go2(inst, std::forward<Tail>(xs)...);
  }

  // Base cases: either there are no SSATmps, or there's a variadic
  // list of SSATmp*'s, or there's an int followed by a SSATmp**.

  Ret go2(IRInstruction* inst) { return stop(inst); }

  template<class... SSAs>
  Ret go2(IRInstruction* inst, SSATmp* t1, SSAs... ts) {
    SSATmp* ssas[] = { t1, ts... };
    inst->initializeSrcs(1 + sizeof...(ts), ssas);
    return stop(inst);
  }

  // For each of the optional parameters, there's an overloaded
  // setter:

  void setter(IRInstruction* inst, std::pair<uint32_t,SSATmp**> ssas) {
    inst->initializeSrcs(ssas.first, ssas.second);
  }

  void setter(IRInstruction* inst, Block* target) {
    inst->setTaken(target);
  }

  void setter(IRInstruction* inst, Trace* trace) {
    inst->setTaken(trace ? trace->front() : nullptr);
  }

  void setter(IRInstruction* inst, Type t) {
    inst->setTypeParam(t);
  }

  void setter(IRInstruction* inst, const IRExtraData& extra) {
    /*
     * Taking the address of this temporary seems scary, but actually
     * it is safe: `extra' was forwarded in all the way from the
     * makeInstruction call, but then we bound a const reference to it
     * at go2() when it was the head of the varargs list, so it must
     * last until the end of the full-expression that called that
     * go2().
     *
     * This is long enough for it to outlast our call to func,
     * although the transient IRInstruction actually will outlive it.
     * We null out the extra data in go() before ~IRInstruction runs,
     * though.
     */
    inst->setExtra(const_cast<IRExtraData*>(&extra));
  }

  // Finally we end up here.
  Ret stop(IRInstruction* inst) {
    if (debug) assertOperandTypes(inst);
    return func(inst);
  }

private:
  const Func& func;
};

}

template<class Func, class... Args>
typename std::result_of<Func (IRInstruction*)>::type
makeInstruction(Func func, Args&&... args) {
  typedef typename std::result_of<Func (IRInstruction*)>::type Ret;
  return detail::InstructionBuilder<Ret,Func>(func).go(args...);
}

//////////////////////////////////////////////////////////////////////

/*
 * IRFactory is used for allocating and controlling the lifetime of IR
 * objects.
 */
class IRFactory {
public:
  IRFactory()
    : m_nextBlockId(0)
    , m_nextOpndId(0)
    , m_nextInstId(0)
  {}

  /*
   * Create an IRInstruction with lifetime equivalent to this IRFactory.
   *
   * Arguments are passed in the following format:
   *
   *   gen(Opcode, [IRExtraData&,] [type param,] [exit label,] [srcs ...]);
   *
   * All arguments are optional except the opcode.  `srcs' may be
   * specified either as a list of SSATmp*'s, or as a integer size and
   * a SSATmp**.
   */
  template<class... Args>
  IRInstruction* gen(Args&&... args) {
    return makeInstruction(
      [this] (IRInstruction* inst) { return inst->clone(this); },
      std::forward<Args>(args)...
    );
  }

  template<class T>
  T* cloneInstruction(const T* inst) {
    T* newInst = new (m_arena) T(m_arena, inst,
                                 IRInstruction::IId(m_nextInstId++));
    if (newInst->modifiesStack()) {
      assert(newInst->naryDst());
      // The instruction is an opcode that modifies the stack, returning a new
      // StkPtr.
      int numDsts = 1 + (newInst->hasMainDst() ? 1 : 0);
      SSATmp* dsts = (SSATmp*)m_arena.alloc(numDsts * sizeof(SSATmp));
      for (int dstNo = 0; dstNo < numDsts; ++dstNo) {
        new (&dsts[dstNo]) SSATmp(m_nextOpndId++, newInst, dstNo);
      }
      newInst->setDsts(numDsts, dsts);
    } else {
      newSSATmp(newInst);
    }
    return newInst;
  }

  IRInstruction* defLabel();
  IRInstruction* defLabel(unsigned numDst);
  template<typename T> SSATmp* defConst(T val) {
    ConstData cdata(val);
    return findConst(cdata, typeForConst(val));
  }
  Block* defBlock(const Func* f, IRInstruction*);
  Block* defBlock(const Func* f) {
    return defBlock(f, defLabel());
  }
  Block* defBlock(const Func* f, unsigned numDst) {
    return defBlock(f, defLabel(numDst));
  }

  /*
   * Creates move instrution that moves from src to dst. We can't use gen
   * to create such a move because gen assigns a newly allocated destination
   * SSATmp whereas we want to use the given dst SSATmp.
   */
  IRInstruction* mov(SSATmp* dst, SSATmp* src);

  Arena&   arena()               { return m_arena; }
  uint32_t numTmps() const       { return m_nextOpndId; }
  uint32_t numBlocks() const     { return m_nextBlockId; }
  uint32_t numInsts() const      { return m_nextInstId; }
  CSEHash& getConstTable()       { return m_constTable; }

private:
  SSATmp* findConst(ConstData& cdata, Type t);
  void newSSATmp(IRInstruction* inst) {
    if (!inst->hasDst()) return;
    SSATmp* tmp = new (m_arena) SSATmp(m_nextOpndId++, inst);
    inst->setDst(tmp);
  }

private:
  CSEHash  m_constTable;
  uint32_t m_nextBlockId;
  uint32_t m_nextOpndId;
  uint32_t m_nextInstId;

  // SSATmp and IRInstruction objects are allocated here.
  Arena m_arena;
};

/*
 * Utility to keep a vector of state about each key, indexed by
 * factoryId(key), where key can be an IRInstruction, Block, or SSATmp.
 */
template<class Key, class Info>
struct StateVector {
  typedef std::vector<Info> InfoVector;
  typedef typename InfoVector::iterator iterator;
  typedef typename InfoVector::reference reference;
  typedef typename InfoVector::const_reference const_reference;

  static unsigned factoryId(const IRInstruction* inst) {
    return inst->getIId();
  }
  static unsigned factoryId(const Block* block) { return block->getId(); }
  static unsigned factoryId(const SSATmp* tmp) { return tmp->getId(); }
  static unsigned count(const IRFactory* factory, IRInstruction*) {
    return factory->numInsts();
  }
  static unsigned count(const IRFactory* factory, SSATmp*) {
    return factory->numTmps();
  }
  static unsigned count(const IRFactory* factory, Block*) {
    return factory->numBlocks();
  }
  StateVector(const IRFactory* factory, Info init)
    : m_factory(factory)
    , m_info(count(factory, (Key*)nullptr), init)
    , m_init(init) {
  }
  reference operator[](const Key& k) { return (*this)[&k]; }
  reference operator[](const Key* k) {
    auto id = factoryId(k);
    if (id >= m_info.size()) grow();
    assert(id < m_info.size());
    return m_info[id];
  }
  const_reference operator[](const Key& k) const { return (*this)[&k]; }
  const_reference operator[](const Key* k) const {
    assert(factoryId(k) < m_info.size());
    return m_info[factoryId(k)];
  }
  void reset() {
    for (size_t i = 0, n = m_info.size(); i < n; ++i) {
      m_info[i] = m_init;
    }
    grow();
  }

  iterator begin() { return m_info.begin(); }
  iterator end() { return m_info.end(); }

private:
  void grow() {
    for (size_t i = m_info.size(), n = count(m_factory, (Key*)nullptr);
         i < n; ++i) {
      m_info.push_back(m_init);
    }
  }

private:
  const IRFactory* m_factory;
  InfoVector m_info;
  Info m_init;
};

//////////////////////////////////////////////////////////////////////

}}}

#endif
