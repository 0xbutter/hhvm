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
#ifndef _EMIT_X64_H_
#define _EMIT_X64_H_

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "util/util.h"
#include "util/base.h"
#include "util/atomic.h"
#include "util/trace.h"

/*
 * An experimental macro assembler for x64, that strives for low coupling to
 * the runtime environment.
 *
 * There are more complete assemblers out there; if you use this one
 * yourself, expect not to find all the instructions you wanted to use. You
 * may have to go spelunking in the Intel manuals:
 *
 *   http://www.intel.com/products/processor/manuals/
 *
 * If you're looking for something more fully baked, here are some options
 * to consider:
 *
 *   1. Nanojit or llvm, both of which translate abstract virtual machine
 *      instructions to the native target architecture, or
 *   2. The embedded assemblers from v8, the Sun JVM, etc.
 */

#ifndef __x86_64__
/*
 * Technically, you could use this to generate x86_64 instructions on some
 * other platform, e.g., in a cross-compiler.
 *
 * Most likely, you didn't mean to do this, though.
 */
#error Your architecture is unsupported.
#endif

/*
 * Some members cannot be const because their values aren't known in
 * an initialization list. Like the opposite of the "mutable" keyword.
 * This declares this property to readers.
 */
#define logical_const /* nothing */

namespace HPHP {
namespace x64 {

TRACE_SET_MOD(asmx64);
typedef int register_name_t;

namespace sz {
  static const int nosize = 0;
  static const int byte  = 1;
  static const int word  = 2;
  static const int dword = 4;
  static const int qword = 8;
}

/*
 * Note that CodeAddresses are not const; the whole point is that we intend
 * to mutate them. uint8_t is as good a type as any: instructions are
 * bytes, and pointer arithmetic works correctly for the architecture.
 */
typedef uint8_t* CodeAddress;
typedef uint8_t* Address;

static inline void
atomic_store64(volatile uint64_t* dest, uint64_t value) {
  // gcc on x64 will implement this with a 64-bit store, and
  // normal 64-bit stores don't tear across instruction boundaries
  // assuming all 8 bytes of dest are on the same cacheline.
  *dest = value;
}


Address allocSlab(size_t size);

/*
 * This needs to be a POD type (no user-declared constructors is the most
 * important characteristic) so that it can be made thread-local.
 */
struct DataBlock {
  logical_const Address base;
  Address               frontier;
  size_t                size;

  /*
   * mmap()s in the desired amount of memory. The size member must be set.
   */
  void init();

  /*
   * alloc --
   *
   *   Simple bump allocator.
   *
   * allocAt --
   *
   *   Some clients need to allocate with an externally maintained frontier.
   *   allocAt supports this.
   */
  void* allocAt(size_t &frontierOff, size_t sz, size_t align = 16) {
    uint8_t* frontier = base + frontierOff;
    ASSERT(base && frontier);
    int slop = uintptr_t(frontier) & (align - 1);
    if (slop) {
      int leftInBlock = (align - slop);
      frontier += leftInBlock;
      frontierOff += leftInBlock;
    }
    ASSERT((uintptr_t(frontier) & (align - 1)) == 0);
    frontierOff += sz;
    ASSERT(frontierOff <= size);
    return frontier;
  }

  template<typename T> T* alloc(size_t align = 16) {
    size_t frontierOff = frontier - base;
    T* retval = (T*)allocAt(frontierOff, sizeof(T), align);
    frontier = base + frontierOff;
    return retval;
  }


  bool canEmit(size_t nBytes) {
    ASSERT(frontier >= base);
    ASSERT(frontier <= base + size);
    return frontier + nBytes <= base + size;
  }

  bool isValidAddress(const CodeAddress tca) {
    return tca >= base && tca < (frontier + size);
  }

  void byte(const uint8_t byte) {
    ASSERT(canEmit(sz::byte));
    TRACE(10, "%p b : %02x\n", frontier, byte);
    *frontier = byte;
    frontier += sz::byte;
  }
  void word(const uint16_t word) {
    ASSERT(canEmit(sz::word));
    *(uint16_t*)frontier = word;
    TRACE(10, "%p w : %04x\n", frontier, word);
    frontier += sz::word;
  }
  void dword(const uint32_t dword) {
    ASSERT(canEmit(sz::dword));
    TRACE(10, "%p d : %08x\n", frontier, dword);
    *(uint32_t*)frontier = dword;
    frontier += sz::dword;
  }
  void qword(const uint64_t qword) {
    ASSERT(canEmit(sz::qword));
    TRACE(10, "%p q : %016lx\n", frontier, qword);
    *(uint64_t*)frontier = qword;
    frontier += sz::qword;
  }

  void bytes(size_t n, const uint8_t *bs) {
    ASSERT(canEmit(n));
    TRACE(10, "%p [%ld b] : [%p]\n", frontier, n, bs);
    if (n <= 8) {
      // If it is a modest number of bytes, try executing in one machine
      // store. This allows control-flow edges, including nop, to be
      // appear idempotent on other CPUs.
      union {
        uint64_t qword;
        uint8_t bytes[8];
      } u;
      u.qword = *(uint64_t*)frontier;
      for (size_t i = 0; i < n; ++i) {
        u.bytes[i] = bs[i];
      }
      atomic_store64((uint64_t*)frontier, u.qword);
    } else {
      memcpy(frontier, bs, n);
    }
    frontier += n;
  }

  protected:
  void makeExecable();

  void *rawBytes(size_t n) {
    void* retval = (void*) frontier;
    frontier += n;
    return retval;
  }
};

/*
 * This is sugar on top of DataBlock, providing a constructor (see
 * DataBlock's comment for why it can't provide constructors itself) and
 * making the allocated memory executable.
 *
 * We seqeuntially pour code into a codeblock from beginning to end.
 * Managing entry points, ensuring the block is big enough, keeping track
 * of cross-codeblock references in code and data, etc., is beyond the
 * scope of this module.
 */
struct CodeBlock : public DataBlock {

  CodeBlock() {};

  /*
   * Allocate executable memory of the specified size, anywhere in
   * the address space.
   */
  void initCodeBlock(size_t sz);

  /*
   * User has pre-allocated the memory. This constructor might change
   * virtual memory permissions to make this block "+rwx".
   */
  void initCodeBlock(CodeAddress start, size_t len);
};

namespace reg {
  static const int noreg = -1;
  static const int rax = 0;
  static const int rcx = 1;
  static const int rdx = 2;
  static const int rbx = 3;
  static const int rsp = 4;
  static const int rbp = 5;
  static const int rsi = 6;
  static const int rdi = 7;

  static const int r8  = 8;
  static const int r9  = 9;
  static const int r10 = 10;
  /*
   * rScratch is a symbolic name for a register that is always free. The
   * ABI is silent about this register, other than to say that it is callee
   * saved.
   */
  static const int rScratch = r10;

  static const int r11 = 11;
  static const int r12 = 12;
  static const int r13 = 13;
  static const int r14 = 14;
  static const int r15 = 15;

  static const int fsPrefix = 0x64;
  static const int gsPrefix = 0x65;
}

enum instrFlags {
  IF_REVERSE    = 0x0001, // The operand encoding for some instructions are
                          // "backwards" in x64; these instructions are
                          // called "reverse" instructions. There are a few
                          // details about emitting "reverse" instructions:
                          // (1) for the R_M address mode, we use the MR
                          // opcode, (2) for M_R and R address modes, we use
                          // the RM opcode, and (3) for the R_R address mode,
                          // we still use MR opcode, but we have to swap the
                          // first argument and the second argument.

  IF_TWOBYTEOP  = 0x0002, // Some instructions have two byte opcodes. For
                          // these instructions, an additional byte (0x0F) is
                          // emitted before the standard opcode byte.

  IF_JCC        = 0x0004, // instruction is jcc
  IF_IMUL       = 0x0008, // instruction is imul
  IF_HAS_IMM8   = 0x0010, // instruction has an encoding that takes an 8-bit
                          // immediate
  IF_SHIFT      = 0x0020, // instruction is rol, ror, rcl, rcr, shl, shr, sar
  IF_RET        = 0x0040, // instruction is ret
  IF_SHIFTD     = 0x0080, // instruction is shld, shrd
  IF_NO_REXW    = 0x0100, // rexW prefix is not needed
  IF_MOV        = 0x0200, // instruction is mov
  IF_COMPACTR   = 0x0400, // instruction supports compact-R encoding
  IF_RAX        = 0x0800, // instruction supports special rax encoding
  IF_XCHG       = 0x1000, // instruction is xchg
  IF_BYTEREG    = 0x2000, // instruction is movzbq, movsbq, setcc
};

/*
  Address mode to table index map:
      Table index 0 <- R_R / M_R(n) / R_M(r) / R(n)
      Table index 1 <- R_M(n) / M_R(r) / R(r)
      Table index 2 <- I / R_I / M_I / R_R_I / M_R_I / R_M_I
      Table index 3 <- "/digit" value used by the above address modes
      Table index 4 <- special R_I (for rax)
      Table index 5 <- compact-R / none

  (n) - for normal instructions only (IF_REVERSE flag is not set)
  (r) - for reverse instructions only (IF_REVERSE flag is set)
*/

struct X64Instr {
  unsigned char table[6];
  unsigned short flags;
};

//                                    0    1    2    3    4    5     flags
const X64Instr instr_jmp =     { { 0xFF,0xF1,0xE9,0x04,0xE9,0xF1 }, 0x0910 };
const X64Instr instr_call =    { { 0xFF,0xF1,0xE8,0x02,0xE8,0xF1 }, 0x0900 };
const X64Instr instr_push =    { { 0xFF,0xF1,0x68,0x06,0xF1,0x50 }, 0x0510 };
const X64Instr instr_pop =     { { 0x8F,0xF1,0xF1,0x00,0xF1,0x58 }, 0x0500 };
const X64Instr instr_inc =     { { 0xFF,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_dec =     { { 0xFF,0xF1,0xF1,0x01,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_not =     { { 0xF7,0xF1,0xF1,0x02,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_neg =     { { 0xF7,0xF1,0xF1,0x03,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_add =     { { 0x01,0x03,0x81,0x00,0x05,0xF1 }, 0x0810 };
const X64Instr instr_sub =     { { 0x29,0x2B,0x81,0x05,0x2D,0xF1 }, 0x0810 };
const X64Instr instr_and =     { { 0x21,0x23,0x81,0x04,0x25,0xF1 }, 0x0810 };
const X64Instr instr_or  =     { { 0x09,0x0B,0x81,0x01,0x0D,0xF1 }, 0x0810 };
const X64Instr instr_xor =     { { 0x31,0x33,0x81,0x06,0x35,0xF1 }, 0x0810 };
const X64Instr instr_mov =     { { 0x89,0x8B,0xC7,0x00,0xF1,0xB8 }, 0x0200 };
const X64Instr instr_test =    { { 0x85,0x85,0xF7,0x00,0xA9,0xF1 }, 0x0800 };
const X64Instr instr_cmp =     { { 0x39,0x3B,0x81,0x07,0x3D,0xF1 }, 0x0810 };
const X64Instr instr_sbb =     { { 0x19,0x1B,0x81,0x03,0x1D,0xF1 }, 0x0810 };
const X64Instr instr_adc =     { { 0x11,0x13,0x81,0x02,0x15,0xF1 }, 0x0810 };
const X64Instr instr_lea =     { { 0xF1,0x8D,0xF1,0x00,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_xchg =    { { 0x87,0x87,0xF1,0x00,0xF1,0x90 }, 0x1000 };
const X64Instr instr_imul =    { { 0xAF,0xF7,0x69,0x05,0xF1,0xF1 }, 0x0019 };
const X64Instr instr_mul =     { { 0xF7,0xF1,0xF1,0x04,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_div =     { { 0xF7,0xF1,0xF1,0x06,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_idiv =    { { 0xF7,0xF1,0xF1,0x07,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_cdq =     { { 0xF1,0xF1,0xF1,0x00,0xF1,0x99 }, 0x0400 };
const X64Instr instr_ret =     { { 0xF1,0xF1,0xC2,0x00,0xF1,0xC3 }, 0x0540 };
const X64Instr instr_jcc =     { { 0xF1,0xF1,0x80,0x00,0xF1,0xF1 }, 0x0114 };
const X64Instr instr_cmovcc =  { { 0x40,0x40,0xF1,0x00,0xF1,0xF1 }, 0x0003 };
const X64Instr instr_setcc =   { { 0x90,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x2102 };
const X64Instr instr_movswq =  { { 0xBF,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0003 };
const X64Instr instr_movzwq =  { { 0xB7,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0003 };
const X64Instr instr_movsbq =  { { 0xBE,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x2003 };
const X64Instr instr_movzbq =  { { 0xB6,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x2003 };
const X64Instr instr_cwde =    { { 0xF1,0xF1,0xF1,0x00,0xF1,0x98 }, 0x0400 };
const X64Instr instr_rol =     { { 0xD3,0xF1,0xC1,0x00,0xF1,0xF1 }, 0x0020 };
const X64Instr instr_ror =     { { 0xD3,0xF1,0xC1,0x01,0xF1,0xF1 }, 0x0020 };
const X64Instr instr_rcl =     { { 0xD3,0xF1,0xC1,0x02,0xF1,0xF1 }, 0x0020 };
const X64Instr instr_rcr =     { { 0xD3,0xF1,0xC1,0x03,0xF1,0xF1 }, 0x0020 };
const X64Instr instr_shl =     { { 0xD3,0xF1,0xC1,0x04,0xF1,0xF1 }, 0x0020 };
const X64Instr instr_shr =     { { 0xD3,0xF1,0xC1,0x05,0xF1,0xF1 }, 0x0020 };
const X64Instr instr_sar =     { { 0xD3,0xF1,0xC1,0x07,0xF1,0xF1 }, 0x0020 };
const X64Instr instr_xadd =    { { 0xC1,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0002 };
const X64Instr instr_cmpxchg = { { 0xB1,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0002 };
const X64Instr instr_nop =     { { 0xF1,0xF1,0xF1,0x00,0xF1,0x90 }, 0x0500 };
const X64Instr instr_shld =    { { 0xA5,0xF1,0xA4,0x00,0xF1,0xF1 }, 0x0082 };
const X64Instr instr_shrd =    { { 0xAD,0xF1,0xAC,0x00,0xF1,0xF1 }, 0x0082 };
const X64Instr instr_int3 =    { { 0xF1,0xF1,0xF1,0x00,0xF1,0xCC }, 0x0500 };

enum ConditionCode {
  CC_E    = 0x04,
  CC_Z    = 0x04,
  CC_NE   = 0x05,
  CC_NZ   = 0x05,
  CC_B    = 0x02,
  CC_NAE  = 0x02,
  CC_BE   = 0x06,
  CC_NA   = 0x06,
  CC_A    = 0x07,
  CC_NBE  = 0x07,
  CC_AE   = 0x03,
  CC_NB   = 0x03,
  CC_NC   = 0x03,
  CC_L    = 0x0C,
  CC_NGE  = 0x0C,
  CC_LE   = 0x0E,
  CC_NG   = 0x0E,
  CC_G    = 0x0F,
  CC_NLE  = 0x0F,
  CC_GE   = 0x0D,
  CC_NL   = 0x0D,
  CC_S    = 0x08,
  CC_NS   = 0x09,
  CC_O    = 0x00,
  CC_NO   = 0x01,
  CC_P    = 0x0A,
  CC_NP   = 0x0B
};

/*
 * When selecting encodings, we often need to assess a two's complement
 * distance to see if it fits in a shorter encoding.
 */
inline bool deltaFits(int64_t delta, int s) {
  // sz::qword is always true
  ASSERT(s == sz::byte ||
         s == sz::word ||
         s == sz::dword);
  int bits = s * 8;
  /*
   * This can probably be improved; gcc can handle the multiplication,
   * but a bitwise test on (1ull << s * 8) would probably avoid a branch.
   */
  return delta < (1ll << (bits-1)) && delta >= -(1ll << (bits-1));
}

///////////////////////////////////////////////////////////////////////////////
// License for Andrew J. Paroski's x86 machine code emitter

/**
 * Copyright (c) 2009, Andrew J. Paroski
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The names of the contributors may not be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL ANDREW J. PAROSKI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

struct X64Assembler {
  CodeBlock code;

  // must use init() later
  X64Assembler() { }

  void init(size_t sz);
  void init(CodeAddress start, size_t sz);

  public:
  void byte(uint8_t b) {
    code.byte(b);
  }
  void word(uint16_t w) {
    code.word(w);
  }
  void dword(uint32_t dw) {
    code.dword(dw);
  }
  void qword(uint64_t qw) {
    code.qword(qw);
  }
  void bytes(size_t n, const uint8_t* bs) {
    code.bytes(n, bs);
  }

  inline int computeImmediateSize(X64Instr op, ssize_t imm)
      __attribute__((noinline)) {
    // Most instructions take a 32-bit immediate, except
    // for ret which takes a 16-bit immediate
    int immSize = sz::dword;
    if (op.flags & IF_RET) {
      immSize = sz::word;
    }
    // Use an 8-bit immediate if the instruction supports it and if
    // the immediate value fits in a byte
    if (deltaFits(imm, sz::byte) && (op.flags & IF_HAS_IMM8) != 0) {
      immSize = sz::byte;
    }
    return immSize;
  }

  inline int computeImmediateSizeForMovRI64(X64Instr op, ssize_t imm)
    ALWAYS_INLINE {
    // The mov instruction supports an 8 byte immediate for
    // the RI address mode when opSz is qword
    if (!deltaFits(imm, sz::dword)) {
      return sz::qword;
    }
    int immSize = sz::dword;
    if (deltaFits(imm, sz::byte) && (op.flags & IF_HAS_IMM8) != 0) {
      immSize = sz::byte;
    }
    return immSize;
  }

  inline void emitImmediate(X64Instr op, ssize_t imm, int immSize)
      ALWAYS_INLINE {
    if (immSize == sz::nosize) {
      return;
    }
    if ((op.flags & (IF_SHIFT | IF_SHIFTD)) == 0) {
      if (immSize == sz::dword) {
        dword(imm);
      } else if (immSize == sz::byte) {
        byte(imm);
      } else if (immSize == sz::word) {
        word(imm);
      } else {
        qword(imm);
      }
    } else {
      // byte immediate between 0 and 31 for shift/rotate instructions
      byte(imm & 0x1f);
    }
  }

  inline void emitModrm(int x, int y, int z) ALWAYS_INLINE {
    byte((x << 6) | ((y & 7) << 3) | (z & 7));
  }

  // op %r
  // ------
  // Restrictions:
  //     r cannot be set to 'none'
  inline void emitCR(X64Instr op, int jcond, int r, int opSz = sz::qword)
      ALWAYS_INLINE {
    ASSERT(r != reg::noreg);
    // REX
    unsigned char rex = 0;
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
    // setcc is special because it writes to a byte register
    if ((op.flags & IF_BYTEREG) != 0 && r >= 4 && r <= 7) rex |= 0x40;
    if (r & 8) rex |= 1;
    if (rex) byte(0x40 | rex);
    // If the instruction supports compact-R mode, use that
    if (op.flags & IF_COMPACTR) {
      byte(op.table[5] | (r & 7));
      return;
    }
    char opcode = (op.flags & IF_REVERSE) ? op.table[1] : op.table[0];
    char rval = op.table[3];
    // Handle two byte opcodes
    if (op.flags & IF_TWOBYTEOP) byte(0x0F);
    byte(opcode | jcond);
    emitModrm(3, rval, r);
  }

  void emitR(X64Instr op, int r, int opSz = sz::qword)
      ALWAYS_INLINE {
    emitCR(op, 0, r, opSz);
  }

  void emitR32(X64Instr op, int r) ALWAYS_INLINE {
    emitCR(op, 0, r, sz::dword);
  }

  // op %r2, %r1
  // -----------
  // Restrictions:
  //     r1 cannot be set to 'reg::noreg'
  //     r2 cannot be set to 'reg::noreg'
  void emitCRR(X64Instr op, int jcond, int r1, int r2, int opSz = sz::qword)
      ALWAYS_INLINE {
    ASSERT(r1 != reg::noreg && r2 != reg::noreg);
    bool reverse = ((op.flags & IF_REVERSE) != 0);
    // The xchg instruction is special
    if (op.flags & IF_XCHG) {
      if (r1 == reg::rax) {
        // REX
        unsigned char rex = 0;
        if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
        // movzbq and movsbq are special because they read from a byte register
        if ((op.flags & IF_BYTEREG) != 0 && r1 >= 4 && r1 <= 7) rex |= 0x40;
        if (r2 & 8) rex |= (reverse ? 4 : 1);
        if (rex) byte(0x40 | rex);
        // If the second register is rax, emit opcode with the first
        // register id embedded
        byte(op.table[5] | (r2 & 7));
        return;
      } else if (r2 == reg::rax) {
        reverse = !reverse;
        // REX
        unsigned char rex = 0;
        if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) {
          rex |= 8;
        }
        if (r1 & 8) rex |= (reverse ? 1 : 4);
        if (rex) byte(0x40 | rex);
        // If the first register is rax, emit opcode with the second
        // register id embedded
        byte(op.table[5] | (r1 & 7));
        return;
      }
    }
    // REX
    unsigned char rex = 0;
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
    if (r1 & 8) rex |= (reverse ? 1 : 4);
    if (r2 & 8) rex |= (reverse ? 4 : 1);
    if (rex) byte(0x40 | rex);
    // For two byte opcodes
    if ((op.flags & (IF_TWOBYTEOP | IF_IMUL)) != 0) byte(0x0F);
    byte(op.table[0] | jcond);
    if (reverse) {
      emitModrm(3, r2, r1);
    } else {
      emitModrm(3, r1, r2);
    }
  }

  void emitCRR32(X64Instr op, int jcond, int r1, int r2)
      ALWAYS_INLINE {
    emitCRR(op, jcond, r1, r2, sz::dword);
  }

  void emitRR(X64Instr op, int r1, int r2, int opSz = sz::qword)
      ALWAYS_INLINE {
    emitCRR(op, 0, r1, r2, opSz);
  }

  void emitRR32(X64Instr op, int r1, int r2) ALWAYS_INLINE {
    emitCRR(op, 0, r1, r2, sz::dword);
  }

  // op $imm, %r
  // -----------
  // Restrictions:
  //     r cannot be set to 'reg::noreg'
  void emitIR(X64Instr op, int r, ssize_t imm, int opSz = sz::qword)
      ALWAYS_INLINE {
    ASSERT(r != reg::noreg);
    // REX
    unsigned char rex = 0;
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
    if (r & 8) rex |= 1;
    if (rex) byte(0x40 | rex);
    // Determine the size of the immediate
    int immSize;
    if ((op.flags & IF_MOV) && opSz == sz::qword) {
      immSize = computeImmediateSizeForMovRI64(op, imm);
    } else {
      immSize = computeImmediateSize(op, imm);
    }
    // Use the special rax encoding if the instruction supports it
    if (r == reg::rax && immSize == sz::dword && (op.flags & IF_RAX)) {
      byte(op.table[4]);
      emitImmediate(op, imm, immSize);
      return;
    }
    // Use the compact-R encoding if this is a mov instruction with
    // a 64-bit immediate
    if (immSize == sz::qword && (op.flags & IF_MOV)) {
      byte(op.table[5] | (r & 7));
      emitImmediate(op, imm, immSize);
      return;
    }
    int rval = op.table[3];
    // shift/rotate instructions have special opcode when
    // immediate is 1
    if ((op.flags & IF_SHIFT) != 0 && imm == 1) {
      byte(0xd1);
      emitModrm(3, rval, r);
      // don't emit immediate
      return;
    }
    int opcode = (immSize != sz::byte) ? op.table[2] : (op.table[2] | 2);
    byte(opcode);
    emitModrm(3, rval, r);
    emitImmediate(op, imm, immSize);
  }

  void emitIR32(X64Instr op, int r, ssize_t imm)
      ALWAYS_INLINE {
    emitIR(op, r, imm, sz::dword);
  }

  // op $imm, %r2, %r1
  // -----------------
  // Restrictions:
  //     r1 cannot be set to 'reg::noreg'
  //     r2 cannot be set to 'reg::noreg'
  void emitIRR(X64Instr op, int r1, int r2, ssize_t imm, int opSz = sz::qword)
      ALWAYS_INLINE {
    ASSERT(r1 != reg::noreg && r2 != reg::noreg);
    bool reverse = ((op.flags & IF_REVERSE) != 0);
    // REX
    unsigned char rex = 0;
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
    if (r1 & 8) rex |= (reverse ? 1 : 4);
    if (r2 & 8) rex |= (reverse ? 4 : 1);
    if (rex) byte(0x40 | rex);
    // Determine the size of the immediate
    int immSize = computeImmediateSize(op, imm);
    // Use 2-byte opcode for cmovcc, setcc, movsx, movzx, movsx8, movzx8
    // instructions
    if ((op.flags & IF_TWOBYTEOP) != 0) byte(0x0F);
    int opcode = (immSize != sz::byte) ? op.table[2] : (op.table[2] | 2);
    byte(opcode);
    if (reverse) {
      emitModrm(3, r2, r1);
    } else {
      emitModrm(3, r1, r2);
    }
    emitImmediate(op, imm, immSize);
  }

  void emitCI(X64Instr op, int jcond, ssize_t imm, int opSz = sz::qword)
      ALWAYS_INLINE {
    // REX
    if ((op.flags & IF_NO_REXW) == 0) {
      byte(0x48);
    }
    // Determine the size of the immediate
    int immSize = computeImmediateSize(op, imm);
    // Emit opcode
    if ((op.flags & IF_JCC) != 0) {
      // jcc is weird so we handle it separately
      if (immSize != sz::byte) {
        byte(0x0F);
        byte(jcond | 0x80);
      } else {
        byte(jcond | 0x70);
      }
    } else {
      int opcode = (immSize != sz::byte) ?
        op.table[2] : (op.table[2] | 2);
      byte(jcond | opcode);
    }
    emitImmediate(op, imm, immSize);
  }

  void emitI(X64Instr op, ssize_t imm, int opSz = sz::qword)
      ALWAYS_INLINE {
    emitCI(op, 0, imm, opSz);
  }

  void emitJ8(X64Instr op, ssize_t imm)
    ALWAYS_INLINE {
    ASSERT((op.flags & IF_JCC) == 0);
    int8_t delta = imm - ((ssize_t)code.frontier + 2);
    // Emit opcode and 8-bit immediate
    byte(0xEB);
    byte(delta);
  }

  void emitCJ8(X64Instr op, int jcond, ssize_t imm)
    ALWAYS_INLINE {
    // this is for jcc only
    ASSERT(op.flags & IF_JCC);
    int8_t delta = imm - ((ssize_t)code.frontier + 2);
    // Emit opcode
    byte(jcond | 0x70);
    // Emit 8-bit offset
    byte(delta);
  }

  void emitJ32(X64Instr op, ssize_t imm) ALWAYS_INLINE {
    // call and jmp are supported, jcc is not supported
    ASSERT((op.flags & IF_JCC) == 0);
    ssize_t delta = imm - ((ssize_t)code.frontier + 5);
    uint8_t *bdelta = (uint8_t*)&delta;
    uint8_t instr[] = { op.table[2],
      bdelta[0], bdelta[1], bdelta[2], bdelta[3] };
    bytes(5, instr);
  }

  void emitCJ32(X64Instr op, int jcond, ssize_t imm)
      ALWAYS_INLINE {
    // jcc is supported, call and jmp are not supported
    ASSERT(op.flags & IF_JCC);
    ssize_t delta = imm - ((ssize_t)code.frontier + 6);
    char* bdelta = (char*)&delta;
    uint8_t instr[6] = { 0x0f, 0x80 | jcond,
      bdelta[0], bdelta[1], bdelta[2], bdelta[3] };
    bytes(6, instr);
  }

  // op disp(%br,%ir,s)
  //   (for reverse == false, hasImmediate == false, r == reg::noreg)
  // op $imm, disp(%br,%ir,s)
  //   (for reverse == false, hasImmediate == true,  r == reg::noreg)
  // op %r, disp(%br,%ir,s)
  //   (for reverse == false, hasImmediate == false, r != reg::noreg)
  // op $imm, %r, disp(%br,%ir,s)
  //   (for reverse == false, hasImmediate == true,  r != reg::noreg)
  // op disp(%br,%ir,s), %r
  //   (for reverse == true,  hasImmediate == false, r != reg::noreg)
  // op $imm, disp(%br,%ir,s), %r
  //   (for reverse == true,  hasImmediate == true,  r != reg::noreg)
  // -----------------------------------------------------------------
  // Restrictions:
  //     ir cannot be set to 'sp'
  void emitCMX(X64Instr op, int jcond, int br, int ir, int s, int disp, int r,
      bool reverse, ssize_t imm, bool hasImmediate, int opSz = sz::qword,
      bool ripRelative = false)
      ALWAYS_INLINE {
    ASSERT(ir != reg::rsp);
    // Determine immSize from the 'hasImmediate' flag
    int immSize = sz::nosize;
    if (hasImmediate) {
      immSize = computeImmediateSize(op, imm);
    }
    if ((op.flags & IF_REVERSE) != 0) reverse = !reverse;
    // Determine if we need to use a two byte opcode;
    // imul is weird so we have a special case for it
    bool twoByteOpcode = ((op.flags & IF_TWOBYTEOP) != 0) ||
      ((op.flags & IF_IMUL) != 0 && r != reg::noreg && immSize == sz::nosize);
    // Again, imul is weird
    if ((op.flags & IF_IMUL) != 0 && r != reg::noreg) reverse = !reverse;
    // The wily rex byte, a multipurpose extension to the opcode space for x64
    unsigned char rex = 0;
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
    // movzbq and movsbq are special because they read from a byte register
    if (reverse == false && r != reg::noreg &&
        (op.flags & IF_BYTEREG) != 0 && r >= 4 && r <= 7) {
      rex |= 0x40;
    }
    if (r != reg::noreg && (r & 8)) rex |= 4;
    if (ir != reg::noreg && (ir & 8)) rex |= 2;
    if (br != reg::noreg && (br & 8)) rex |= 1;
    if (rex) byte(0x40 | rex);
    // Emit the opcode
    if (immSize != sz::nosize) {
      if (twoByteOpcode) byte(0x0F);
      if (immSize == sz::byte) {
        byte(op.table[2] | 2 | jcond);
      } else {
        byte(op.table[2] | jcond);
      }
    } else {
      if (twoByteOpcode) byte(0x0F);
      int opcode;
      if ((op.flags & IF_IMUL) != 0) {
        opcode = (r == reg::noreg) ? op.table[1] : op.table[0];
      } else {
        opcode = reverse ? op.table[1] : op.table[0];
      }
      byte(opcode | jcond);
    }
    // SIB byte if:
    //   1. We're using an index register.
    //   2. The base register is rsp-like.
    //   3. We're doing a baseless disp access and it is not rip-relative.
    bool sibIsNeeded =
      ir != reg::noreg ||                 /* 1 */
      br == reg::rsp || br == reg::r12 || /* 2 */
      (br == reg::noreg && !ripRelative);
    // If there is no register and no immediate, use the /r value
    if (r == reg::noreg) r = op.table[3];
    // If 'reg::noreg' was specified for 'ir', we use
    // the encoding for the sp register
    if (ir == reg::noreg) ir = 4;
    int dispSize = sz::nosize;
    if (disp != 0) {
      if (disp <= 127 && disp >= -128) {
        dispSize = sz::byte;
      } else {
        dispSize = sz::dword;
      }
    }
    // Set 'mod' based on the size of the displacement
    int mod;
    switch (dispSize) {
      case sz::nosize: mod = 0; break;
      case sz::byte: mod = 1; break;
      default: mod = 2; break;
    }
    // Handle special cases for 'br'
    if (br == reg::noreg) {
      // If 'reg::noreg' was specified for 'br', we use the
      // encoding for the bp register, and we must set
      // mod=0 and "upgrade" to a DWORD-sized displacement
      br = 5;
      mod = 0;
      dispSize = sz::dword;
    } else if ((br & 7) == 5 && dispSize == sz::nosize) {
      // If br == rbp and no displacement was specified, we
      // must "upgrade" to using a 1-byte displacement value
      dispSize = sz::byte;
      mod = 1;
    }
    // Emit modr/m and the sib
    if (sibIsNeeded) {
      // s:                               0  1  2   3  4   5   6   7  8
      static const int scaleLookup[] = { -1, 0, 1, -1, 2, -1, -1, -1, 3 };
      ASSERT(s > 0 && s <= 8);
      int scale = scaleLookup[s];
      ASSERT(scale != -1);
      emitModrm(mod, r, 4);
      byte((scale << 6) | ((ir & 7) << 3) | (br & 7));
    } else {
      emitModrm(mod, r, br);
    }
    // Emit displacement if needed
    if (dispSize == sz::dword) {
      dword(disp);
    } else if (dispSize == sz::byte) {
      byte(disp & 0xff);
    }
    // Emit immediate if needed
    if (immSize != sz::nosize) {
      emitImmediate(op, imm, immSize);
    }
  }

  void emitIM(X64Instr op, int br, int ir, int s, int disp, ssize_t imm,
      int opSz = sz::qword) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, reg::noreg, false, imm, true, opSz);
  }

  void emitIM32(X64Instr op, int br, int ir, int s, int disp, ssize_t imm)
      ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, reg::noreg, false, imm, true, sz::dword);
  }

  void emitRM(X64Instr op, int br, int ir, int s, int disp, int r,
      int opSz = sz::qword) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, r, false, 0, false, opSz);
  }

  void emitRM32(X64Instr op, int br, int ir, int s, int disp, int r)
      ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, r, false, 0, false, sz::dword);
  }

  void emitCMR(X64Instr op, int jcond, int br, int ir, int s, int disp, int r,
      int opSz = sz::qword) ALWAYS_INLINE {
    emitCMX(op, jcond, br, ir, s, disp, r, true, 0, false, opSz);
  }

  void emitMR(X64Instr op, int br, int ir, int s, int disp, int r,
      int opSz = sz::qword) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, r, true, 0, false, opSz);
  }

  void emitMR32(X64Instr op, int br, int ir, int s, int disp, int r)
      ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, r, true, 0, false, sz::dword);
  }

  void emitIRM(X64Instr op, int br, int ir, int s, int disp, int r,
      ssize_t imm, int opSz = sz::qword) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, r, false, imm, true, opSz);
  }

  void emitIMR(X64Instr op, int br, int ir, int s, int disp, int r,
      ssize_t imm, int opSz = sz::qword) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, r, true, imm, true, opSz);
  }

  void emitM(X64Instr op, int br, int ir, int s, int disp,
      int opSz = sz::qword) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, reg::noreg, false, 0, false, opSz);
  }

  void emitM32(X64Instr op, int br, int ir, int s, int disp)
      ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, reg::noreg, false, 0, false, sz::dword);
  }

  void emitCM(X64Instr op, int jcond, int br, int ir, int s, int disp,
      int opSz = sz::qword) ALWAYS_INLINE {
    emitCMX(op, jcond, br, ir, s, disp, reg::noreg, false, 0, false, opSz);
  }

  // emit (with no arguments)
  void emit(X64Instr op) ALWAYS_INLINE {
    if ((op.flags & IF_NO_REXW) == 0) {
      byte(0x48);
    }
    byte(op.table[5]);
  }

  void emitSRPrefix(int segr) ALWAYS_INLINE {
    ASSERT(segr == reg::fsPrefix || segr == reg::gsPrefix);
    byte(segr);
  }

  void fs() {
    emitSRPrefix(reg::fsPrefix);
  }

  void emitLockPrefix() {
    byte(0xF0);
  }

  void emitInt3s(int n) {
    memset(code.frontier, 0xcc, n);
    code.frontier += n;
  }

  void emitNop(int n) {
    const uint8_t nops[][9] = {
      { },
      { 0x90 },
      { 0x66, 0x90 },
      { 0x0f, 0x1f, 0x00 },
      { 0x0f, 0x1f, 0x40, 0x00 },
      { 0x0f, 0x1f, 0x44, 0x00, 0x00 },
      { 0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00 },
      { 0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00 },
      { 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 },
      { 0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 },
    };
    // While n >= 9, emit 9 byte NOPs
    while (n >= 9) {
      bytes(9, nops[9]);
      n -= 9;
    }
    bytes(n, nops[n]);
  }

  public:
  /*
   * Our ordering convention follows the gas standard of "destination
   * last": <op>_<src1>_<src2>_<dest>. Be warned that Intel manuals go the
   * other way; in practice it's more important to be consistent with the
   * tools (gdb, gas, inline asm, etc.) than the manuals, since you look at
   * the former an order of magnitude more.
   */

  inline void mov_imm64_reg(int64_t imm, int rn) {
    emitIR(instr_mov, rn, imm);
  }

  inline void mov_imm32_reg32(int32_t imm, int rn) {
    emitIR(instr_mov, rn, imm, sz::dword);
  }

  inline void store_reg32_disp_reg64(int rsrc, int off, int rdest) {
    emitRM32(instr_mov, rdest, reg::noreg, sz::byte, off, rsrc);
  }

  inline void load_reg64_disp_reg32(register_name_t rsrc, int off,
                                    register_name_t rdest) {
    emitMR32(instr_mov, rsrc, reg::noreg, sz::byte, off, rdest);
  }

  inline void load_disp32_reg64(int disp, register_name_t rdest) {
    emitMR(instr_mov, reg::noreg, reg::noreg, sz::byte, disp, rdest);
  }

  inline void store_imm32_disp_reg(int imm, int off, int rdest) {
    emitIM32(instr_mov, rdest, reg::noreg, sz::byte, off, imm);
  }

  inline void shl_reg64(int imm, int rsrc) {
    emitIR(instr_shl, rsrc, imm, sz::dword);
  }

  inline void mov_reg64_reg64(int rsrc, int rdest) {
    emitRR(instr_mov, rsrc, rdest);
  }

  inline void mov_reg32_reg32(int rsrc, int rdest) {
    emitRR32(instr_mov, rsrc, rdest);
  }

  inline void store_imm_disp_reg64(int64_t imm, int off, int rdest) {
    mov_imm64_reg(imm, reg::rScratch);
    emitRM(instr_mov, rdest, reg::noreg, sz::byte, off, reg::rScratch);
  }

  // mov %rsrc, disp(%rdest)
  inline void store_reg64_disp_reg64(int rsrc, int off, int rdest) {
    emitRM(instr_mov, rdest, reg::noreg, sz::byte, off, rsrc);
  }

  // mov disp(%rsrc), %rdest
  inline void load_reg64_disp_reg64(register_name_t rsrc, int off,
                                    register_name_t rdest) {
    emitMR(instr_mov, rsrc, reg::noreg, sz::byte, off, rdest);
  }

  /*
   * Control-flow directives. The labeling/patching facilities
   * available are primitive.
   */

  /*
   * Jump to an absolute address. May emit code that stomps rScratch.
   */
  inline void jmp(CodeAddress dest) {
    int64_t delta = dest - (code.frontier + 5);
    if (!deltaFits(delta, sz::dword)) {
      mov_imm64_reg   ((uint64_t)dest, reg::rScratch);
      jmp_reg        (reg::rScratch);
      return;
    }
    emitJ32(instr_jmp, (ssize_t)dest);
  }

  inline void jmp8(CodeAddress dest) {
    emitJ8(instr_jmp, (ssize_t)dest);
  }

  inline void jmp_reg(int rn) {
    emitR(instr_jmp, rn);
  }

  /*
   * Call an absolute address. May emit code that stomps rScratch.
   */
  inline void call(CodeAddress dest) {
    int64_t delta = dest - (code.frontier + 5);
    if (!deltaFits(delta, sz::dword)) {
      mov_imm64_reg   ((int64_t)dest, reg::rScratch);
      call_reg        (reg::rScratch);
      return;
    }
    emitJ32(instr_call, (ssize_t)dest);
  }

  inline void call_reg(int rn) {
    emitR(instr_call, rn);
  }

  /*
   * This emits jcc using 32-bit relative offset form.
   */
  inline void jcc(int cond, CodeAddress dest) {
    emitCJ32(instr_jcc, cond, (ssize_t)dest);
  }

  inline void jcc8(int cond, CodeAddress dest) {
    emitCJ8(instr_jcc, cond, (ssize_t)dest);
  }

  // Some instructions are parameterized by cc.
#define CCS \
  CC(o,   CC_O)         \
  CC(no,  CC_NO)        \
  CC(nae, CC_NAE)       \
  CC(nb,  CC_NB)        \
  CC(e,   CC_E)         \
  CC(z,   CC_Z)         \
  CC(ne,  CC_NE)        \
  CC(nz,  CC_NZ)        \
  CC(be,  CC_BE)        \
  CC(nbe, CC_NBE)       \
  CC(s,   CC_S)         \
  CC(ns,  CC_NS)        \
  CC(p,   CC_P)         \
  CC(np,  CC_NP)        \
  CC(nge, CC_NGE)       \
  CC(l,   CC_L)         \
  CC(nl,  CC_NL)        \
  CC(ng,  CC_NG)        \
  CC(le,  CC_LE)        \
  CC(nle, CC_NLE)

#define CC(_nm, _code) \
  inline void j ## _nm(CodeAddress dest) { jcc(_code, dest); }
  CCS
#undef CC

  inline void patchJcc(CodeAddress jmp, CodeAddress dest) {
    ASSERT(jmp[0] == 0x0F && (jmp[1] & 0xF0) == 0x80);
    ssize_t diff = dest - (jmp + 6);
    ASSERT(deltaFits(diff, sz::dword));
    *(int32_t*)(jmp + 2) = (int32_t)diff;
  }

  inline void patchJcc8(CodeAddress jmp, CodeAddress dest) {
    ASSERT((jmp[0] & 0xF0) == 0x70);
    ssize_t diff = dest - (jmp + 2);  // one for opcode, one for offset
    ASSERT(deltaFits(diff, sz::byte));
    *(int8_t*)(jmp + 1) = (int8_t)diff;
  }

  inline void patchJmp(CodeAddress jmp, CodeAddress dest) {
    ASSERT(jmp[0] == 0xE9);
    ssize_t diff = dest - (jmp + 5);
    ASSERT(deltaFits(diff, sz::dword));
    *(int32_t*)(jmp + 1) = (int32_t)diff;
  }

  inline void patchJmp8(CodeAddress jmp, CodeAddress dest) {
    ASSERT(jmp[0] == 0xEB);
    ssize_t diff = dest - (jmp + 2);  // one for opcode, one for offset
    ASSERT(deltaFits(diff, sz::byte));
    *(int8_t*)(jmp + 1) = (int8_t)diff;
  }

  /*
   * We do a lot of calls with constant arguments. Caller is responsible
   * for saving registers, if necessary.
   */
  inline void call1(CodeAddress dest, uintptr_t a1) {
    mov_imm64_reg(a1, reg::rdi);
    call(dest);
  }
  inline void call2(CodeAddress dest, uintptr_t a1, uintptr_t a2) {
    mov_imm64_reg(a2, reg::rsi);
    call1(dest, a1);
  }
  inline void call3(CodeAddress dest, uintptr_t a1, uintptr_t a2, uintptr_t a3) {
    mov_imm64_reg(a3, reg::rdx);
    call2(dest, a1, a2);
  }
  inline void call4(CodeAddress dest, uintptr_t a1, uintptr_t a2, uintptr_t a3,
                    uintptr_t a4) {
    mov_imm64_reg(a4, reg::rcx);
    call3(dest, a1, a2, a3);
  }
  inline void call5(CodeAddress dest, uintptr_t a1, uintptr_t a2, uintptr_t a3,
                    uintptr_t a4, uintptr_t a5) {
    mov_imm64_reg(a5, reg::r8);
    call4(dest, a1, a2, a3, a4);
  }
  inline void call6(CodeAddress dest, uintptr_t a1, uintptr_t a2, uintptr_t a3,
                    uintptr_t a4, uintptr_t a5, uintptr_t a6) {
    mov_imm64_reg(a6, reg::r9);
    call5(dest, a1, a2, a3, a4, a5);
  }

  inline void mov_reg8_reg64_unsigned(int rsrc, int rdest) {
    emitRR(instr_movzbq, rsrc, rdest);
  }

  // lea disp(%rsrc), %rdest
  inline void lea_reg64_disp_reg64(int rsrc, int off, int rdest) {
    emitMR(instr_lea, rsrc, reg::noreg, sz::byte, off, rdest);
  }

  inline void not_reg64(register_name_t rn) {
    emitR(instr_not, rn);
  }

  /*
   * Escaped opcodes for setcc family of instructions; always preceded with
   * lock prefix/opcode escape byte 0x0f for these meanings. Generally if
   * setX tests for condition foo, setX ^ 1 test for condition !foo.
   *
   * Some are aliases: "nge" is the same as "l", and "equal" and "zero" are
   * treated the same on x86.
   */

  inline void setcc(int cc, register_name_t byteReg) {
    emitCR(instr_setcc, cc, byteReg);
  }

#define CC(_nm, _cond)                              \
  inline void set ## _nm(register_name_t byteReg) { \
    setcc(_cond, byteReg); }
  CCS
#undef CC

#define SIMPLE_OP(name)                                                  \
  /* op rsrc, rdest */                                                   \
  inline void name ## _reg64_reg64(int rsrc, int rdest) {                \
    emitRR(instr_ ## name, rsrc, rdest);                                 \
  }                                                                      \
  /* op esrc, edest */                                                   \
  inline void name ## _reg32_reg32(int rsrc, int rdest) {                \
    emitRR32(instr_ ## name, rsrc, rdest);                               \
  }                                                                      \
  /* op imm32, rdest */                                                  \
  inline void name ## _imm32_reg64(int64_t imm, int rdest) {             \
    ASSERT(deltaFits(imm, sz::dword));                                   \
    emitIR(instr_ ## name, rdest, imm);                                  \
  }                                                                      \
  /* op imm32, edest */                                                  \
  inline void name ## _imm32_reg32(int64_t imm, int rdest) {             \
    ASSERT(deltaFits(imm, sz::dword));                                   \
    emitIR32(instr_ ## name, rdest, imm);                                \
  }                                                                      \
  /* opl imm, disp(rdest) */                                             \
  inline void name ## _imm32_disp_reg32(int64_t imm, int disp,           \
                                        int rdest) {                     \
    ASSERT(deltaFits(imm, sz::dword));                                   \
    emitIM32(instr_ ## name, rdest, reg::noreg, sz::byte, disp, imm);    \
  }                                                                      \
  /* opq imm, disp(rdest) */                                             \
  inline void name ## _imm64_disp_reg64(int64_t imm, int disp,           \
                                        int rdest) {                     \
    emitIM(instr_cmp, rdest, reg::noreg, sz::byte, disp, imm);           \
  }                                                                      \
  /* op imm64, rdest */                                                  \
  /* NOTE: This will emit multiple x64 instructions and use the */       \
  /* scratch register if the immediate does not fit in 32 bits. */       \
  inline void name ## _imm64_reg64(int64_t imm, int rdest) {             \
    if (deltaFits(imm, sz::dword)) {                                     \
      name ## _imm32_reg64(imm, rdest);                                  \
      return;                                                            \
    }                                                                    \
    mov_imm64_reg(imm, reg::rScratch);                                   \
    name ## _reg64_reg64(reg::rScratch, rdest);                          \
  }                                                                      \
  /* opq rsrc, disp(rdest) */                                            \
  inline void name ## _reg64_disp_reg64(int rsrc, int disp, int rdest) { \
    emitRM(instr_ ## name, rdest, reg::noreg, sz::byte, disp, rsrc);     \
  }                                                                      \
  /* opl esrc, disp(rdest) */                                            \
  inline void name ## _reg32_disp_reg64(int rsrc, int disp, int rdest) { \
    emitRM32(instr_ ## name, rdest, reg::noreg, sz::byte, disp, rsrc);   \
  }                                                                      \

  SIMPLE_OP(add)
  SIMPLE_OP(xor)
  SIMPLE_OP(sub)
  SIMPLE_OP(and)
  SIMPLE_OP(or)
  SIMPLE_OP(shl)
  SIMPLE_OP(shr)
  SIMPLE_OP(test)
  SIMPLE_OP(cmp)
#undef SIMPLE_OP

  // imul rsrc, rdest
  inline void imul_reg64_reg64(int rsrc, int rdest) {
    emitRR(instr_imul, rsrc, rdest);
  }

  // divisor: register name of divisor.
  // 128-bit dividend is always implicitly in RDX:RAX. Quotient
  // goes to RAX, remainder goes to RDX.
  inline void idiv(int divisor) {
    emitR(instr_idiv, divisor);
  }
  inline void imul(int source) {
    emitR(instr_imul, source);
  }
  inline void pushr(register_name_t reg) {
    emitR(instr_push, reg);
  }
  inline void popr(register_name_t reg) {
    emitR(instr_pop, reg);
  }
  inline void ret() {
    emit(instr_ret);
  }
  inline void nop() {
    emit(instr_nop);
  }
  inline void int3() {
    emit(instr_int3);
  }
  inline void ud2() {
    byte(0x0f); byte(0x0b);
  }

  inline void inc_reg64(register_name_t srcdest) {
    emitR(instr_inc, srcdest);
  }
  inline void inc_reg32(register_name_t srcdest) {
    emitR32(instr_inc, srcdest);
  }
  void inc_mem64(register_name_t addr, int off) {
    emitM(instr_inc, addr, reg::noreg, sz::byte, off);
  }
  inline void inc_mem32(register_name_t addr, int off) {
    emitM32(instr_inc, addr, reg::noreg, sz::byte, off);
  }

  inline void dec_reg64(register_name_t srcdest) {
    emitR(instr_dec, srcdest);
  }
  inline void dec_reg32(register_name_t srcdest) {
    emitR32(instr_dec, srcdest);
  }
  void dec_mem64(register_name_t addr, int off) {
    emitM(instr_dec, addr, reg::noreg, sz::byte, off);
  }
  inline void dec_mem32(register_name_t addr, int off) {
    emitM32(instr_dec, addr, reg::noreg, sz::byte, off);
  }
  inline void pushf() {
    byte(0x9c);
  }
  inline void popf() {
    byte(0x9d);
  }

  inline void xchg_reg64_reg64(register_name_t rsrc, register_name_t rdest) {
    emitRR(instr_xchg, rsrc, rdest);
  }
  inline void xchg_reg32_reg32(register_name_t rsrc, register_name_t rdest) {
    emitRR32(instr_xchg, rsrc, rdest);
  }
};

} } // HPHP::x64

#endif
