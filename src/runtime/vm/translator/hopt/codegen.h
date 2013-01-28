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

#ifndef incl_HPHP_VM_CG_H_
#define incl_HPHP_VM_CG_H_

#include <vector>
#include "runtime/vm/translator/hopt/ir.h"
#include "runtime/vm/translator/targetcache.h"
#include <runtime/vm/translator/translator-x64.h>

namespace HPHP {
namespace VM {
namespace JIT {

using namespace HPHP::VM::Transl;
using namespace HPHP::VM::Transl::TargetCache;

class FailedCodeGen : public std::exception {
 public:
  const char* file;
  const int   line;
  const char* func;
  FailedCodeGen(const char* _file, int _line, const char* _func) :
      file(_file), line(_line), func(_func) { }
};

struct ArgGroup;

enum SyncOptions {
  kNoSyncPoint,
  kSyncPoint,
  kSyncPointAdjustOne,
};

struct CodeGenerator {
  typedef Transl::X64Assembler Asm;

  CodeGenerator(Asm& as, Asm& astubs, Transl::TranslatorX64* tx64) :
      m_as(as), m_astubs(astubs), m_tx64(tx64),
      m_curInst(NULL), m_lastMarker(NULL), m_curTrace(NULL) {
  }

  void cgTrace(Trace* trace, vector<TransBCMapping>* bcMap);

private:
  Address cgInst(IRInstruction* inst);

  // Autogenerate function declarations for each IR instruction in ir.h
#define OPC(name, flags) void cg##name(IRInstruction* inst);
  IR_OPCODES
#undef OPC

  // helper functions for code generation
  void cgCallHelper(Asm&,
                    TCA addr,
                    SSATmp* dst,
                    SyncOptions sync,
                    ArgGroup& args);
  void cgCallHelper(Asm& a,
                    TCA addr,
                    PhysReg dstReg,
                    SyncOptions sync,
                    ArgGroup& args);

  void cgCallHelper(Asm& a,
                    const Transl::Call& call,
                    PhysReg dstReg0,
                    PhysReg dstReg1,
                    SyncOptions sync,
                    ArgGroup& args);

  void cgStore(PhysReg base,
               int64_t off,
               SSATmp* src,
               bool genStoreType = true);
  void cgStoreTypedValue(PhysReg base, int64_t off, SSATmp* src);

  void cgLoad(Type::Tag type,
              SSATmp* dst,
              PhysReg base,
              int64_t off,
              LabelInstruction* label,
              IRInstruction* inst = NULL);

  template<class OpndType>
  void emitGuardType(Type::Tag         type,
                     OpndType          src,
                     LabelInstruction* label,
                     IRInstruction*    instr);

  void cgGuardTypeCell(Type::Tag         type,
                       PhysReg           baseReg,
                       int64_t           offset,
                       LabelInstruction* label,
                       IRInstruction*    instr);

  void cgStMemWork(IRInstruction* inst, bool genStoreType);
  void cgStRefWork(IRInstruction* inst, bool genStoreType);
  void cgStLocWork(IRInstruction* inst, bool genStoreType);
  void cgStPropWork(IRInstruction* inst, bool genStoreType);
  void cgIncRefWork(Type::Tag type, SSATmp* dst, SSATmp* src);
  void cgDecRefWork(IRInstruction* inst, bool genZeroCheck);

  template<class OpInstr, class Oper>
  void cgUnaryIntOp(SSATmp* dst, SSATmp* src, OpInstr, Oper);

  enum Commutativity { Commutative, NonCommutative };
  template<class Oper>
  void cgBinaryIntOp(IRInstruction*,
                     void (Asm::*)(Immed, Reg64),
                     void (Asm::*)(Reg64, Reg64),
                     Oper,
                     Commutativity);

  void cgNegateWork(SSATmp* dst, SSATmp* src);
  void cgNotWork(SSATmp* dst, SSATmp* src);

  void cgLoadTypedValue(Type::Tag type,
                        SSATmp* dst,
                        PhysReg base,
                        int64_t off,
                        LabelInstruction* label,
                        IRInstruction* inst);

  void cgNegate(IRInstruction* inst); // helper
  void cgJcc(IRInstruction* inst); // helper
  void cgOpCmpHelper(
            IRInstruction* inst,
            void (Asm::*setter)(Reg8),
            int64 (*str_cmp_str)(StringData*, StringData*),
            int64 (*str_cmp_int)(StringData*, int64),
            int64 (*str_cmp_obj)(StringData*, ObjectData*),
            int64 (*obj_cmp_obj)(ObjectData*, ObjectData*),
            int64 (*obj_cmp_int)(ObjectData*, int64),
            int64 (*arr_cmp_arr)(ArrayData*, ArrayData*));
  void cgJmpZeroHelper(IRInstruction* inst, ConditionCode cc);


private:
  void emitTraceCall(CodeGenerator::Asm& as, int64 pcOff);
  void emitTraceRet(CodeGenerator::Asm& as);
  void emitCheckStack(CodeGenerator::Asm& as, SSATmp* sp, uint32 numElems,
                      bool allocActRec);
  void emitCheckCell(CodeGenerator::Asm& as,
                     SSATmp* sp,
                     uint32 index);
  Address cgCheckStaticBit(Type::Tag type,
                           PhysReg reg,
                           bool regIsCount);
  Address cgCheckStaticBitAndDecRef(Type::Tag type,
                                    PhysReg dataReg,
                                    LabelInstruction* exit);
  Address cgCheckRefCountedType(PhysReg typeReg);
  Address cgCheckRefCountedType(PhysReg baseReg,
                                int64 offset);
  void cgDecRefStaticType(Type::Tag type,
                          PhysReg dataReg,
                          LabelInstruction* exit,
                          bool genZeroCheck);
  void cgDecRefDynamicType(PhysReg typeReg,
                           PhysReg dataReg,
                           LabelInstruction* exit,
                           bool genZeroCheck);
  void cgDecRefDynamicTypeMem(PhysReg baseReg,
                              int64 offset,
                              LabelInstruction* exit);
  void cgDecRefMem(Type::Tag type,
                   PhysReg baseReg,
                   int64 offset,
                   LabelInstruction* exit);
  void emitSpillActRec(SSATmp* sp,
                       int64_t spOffset,
                       SSATmp* defAR);

  void cgIterNextCommon(IRInstruction* inst, bool isNextK);
  void cgIterInitCommon(IRInstruction* inst, bool isInitK);
  Address emitFwdJcc(ConditionCode cc, LabelInstruction* label);
  Address emitFwdJcc(Asm& a, ConditionCode cc, LabelInstruction* label);
  Address emitFwdJmp(Asm& as, LabelInstruction* label);
  Address emitFwdJmp(LabelInstruction* label);
  Address emitSmashableFwdJmp(LabelInstruction* label, SSATmp* toSmash);
  Address emitSmashableFwdJccAtEnd(ConditionCode cc, LabelInstruction* label,
                              SSATmp* toSmash);
  Address emitSmashableFwdJcc(ConditionCode cc, LabelInstruction* label,
                              SSATmp* toSmash);
  void emitGuardOrFwdJcc(IRInstruction*    inst,
                         ConditionCode     cc,
                         LabelInstruction* label);
  void emitContVarEnvHelperCall(SSATmp* fp, TCA helper);
  const Func* getCurFunc();
  Class*      getCurClass() { return getCurFunc()->cls(); }
  void recordSyncPoint(Asm& as, SyncOptions sync = kSyncPoint);
  Address getDtorGeneric();
  Address getDtorTyped();
  int getIterOffset(SSATmp* tmp);

private:
  /*
   * Fields
   */
  Asm& m_as;
  Asm& m_astubs;
  TranslatorX64* m_tx64;
  // current instruction for which code is being generated
  IRInstruction* m_curInst;
  // the last marker instruction before curInst
  MarkerInstruction* m_lastMarker;
  Trace* m_curTrace;
};

class ArgDesc {
public:
  enum Kind {
    Reg,     // Normal register
    TypeReg, // Type register. Might need arch-specific mangling before call
    Imm,     // Immediate
    Addr,    // Address
  };

  PhysReg getDstReg() const { return m_dstReg; }
  PhysReg getSrcReg() const { return m_srcReg; }
  Kind getKind() const { return m_kind; }
  void setDstReg(PhysReg reg) { m_dstReg = reg; }
  Address genCode(CodeGenerator::Asm& as) const;
  Immed getImm() const { return m_imm; }

private: // These should be created using ArgGroup.
  friend struct ArgGroup;

  explicit ArgDesc(Kind kind, PhysReg srcReg, Immed immVal)
    : m_kind(kind)
    , m_srcReg(srcReg)
    , m_dstReg(reg::noreg)
    , m_imm(immVal)
  {}

  explicit ArgDesc(SSATmp* tmp, bool val = true);

private:
  Kind m_kind;
  PhysReg m_srcReg;
  PhysReg m_dstReg;
  Immed m_imm;
};

/*
 * Bag of ArgDesc for use with cgCallHelper.
 *
 * You can create this using function chaining.  Example:
 *
 *   ArgGroup args;
 *   args.imm(0)
 *       .reg(rax)
 *       .immPtr(StringData::GetStaticString("Yo"))
 *       ;
 *   assert(args.size() == 3);
 */
struct ArgGroup {
  size_t size() const { return m_args.size(); }

  ArgDesc& operator[](size_t i) {
    assert(i < size());
    return m_args[i];
  }

  ArgGroup& imm(uintptr_t imm) {
    m_args.push_back(ArgDesc(ArgDesc::Imm, InvalidReg, imm));
    return *this;
  }

  template<class T> ArgGroup& immPtr(const T* ptr) {
    return imm(uintptr_t(ptr));
  }

  ArgGroup& reg(PhysReg reg) {
    m_args.push_back(ArgDesc(ArgDesc::Reg, PhysReg(reg), -1));
    return *this;
  }

  ArgGroup& type(Type::Tag tag) {
    m_args.push_back(ArgDesc(ArgDesc::Imm, InvalidReg,
                             Type::toDataType(tag)));
    return *this;
  }

  ArgGroup& addr(PhysReg base, intptr_t off) {
    m_args.push_back(ArgDesc(ArgDesc::Addr, PhysReg(base), off));
    return *this;
  }

  ArgGroup& ssa(SSATmp* tmp) {
    m_args.push_back(ArgDesc(tmp));
    return *this;
  }

  ArgGroup& ssas(IRInstruction* inst, unsigned begin, unsigned count) {
    for (SSATmp* s : inst->getSrcs().subpiece(begin, count)) {
      m_args.push_back(ArgDesc(s));
    }
    return *this;
  }

  /* loads the type of tmp into an arg register */
  ArgGroup& type(SSATmp* tmp) {
    m_args.push_back(ArgDesc(tmp, false));
    return *this;
  }

  ArgGroup& valueType(SSATmp* tmp) {
    return ssa(tmp).type(tmp);
  }

private:
  std::vector<ArgDesc> m_args;
};

void genCodeForTrace(Trace*                  trace,
                     CodeGenerator::Asm&     a,
                     CodeGenerator::Asm&     astubs,
                     IRFactory*              irFactory,
                     vector<TransBCMapping>* bcMap,
                     TranslatorX64*          tx64);

}}}

#endif
