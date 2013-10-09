
#include "hphp/vixl/a64/macro-assembler-a64.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/service-requests.h"

namespace HPHP { namespace JIT { namespace ARM {

using namespace vixl;

TCA emitServiceReqWork(CodeBlock& cb, TCA start, bool persist, SRFlags flags,
                       ServiceRequest req, const ServiceReqArgVec& argv) {
  MacroAssembler a { cb };

  assert(start == cb.frontier());

  for (auto i = 0; i < argv.size(); ++i) {
    auto reg = serviceReqArgReg(i);
    auto const& arg = argv[i];
    switch (arg.m_kind) {
      case ServiceReqArgInfo::Immediate:
        a.   Mov  (reg, arg.m_imm);
        break;
      case ServiceReqArgInfo::CondCode:
        not_implemented();
        break;
      default: not_reached();
    }
  }

  // Save VM regs
  a.     Str   (rVmFp, rGContextReg[offsetof(VMExecutionContext, m_fp)]);
  a.     Str   (rVmSp, rGContextReg[offsetof(VMExecutionContext, m_stack) +
                                    Stack::topOfStackOffset()]);

  if (persist) {
    a.   Mov   (rAsm, 0);
  } else {
    a.   Mov   (rAsm, reinterpret_cast<intptr_t>(start));
  }
  a.     Mov   (argReg(0), req);

  // The x64 equivalent loads to rax. I knew this was a trap.
  if (flags & SRFlags::JmpInsteadOfRet) {
    a.   Ldr   (rAsm, MemOperand(sp, 8, PostIndex));
    a.   Br    (rAsm);
  } else {
    a.   Ret   ();
  }
  a.     Brk   (0);

  return start;
}

}}}
