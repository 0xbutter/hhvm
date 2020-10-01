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

#include "hphp/runtime/base/bespoke/layout.h"

#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/punt.h"

#include <atomic>
#include <array>

namespace HPHP { namespace bespoke {

using namespace jit;
using namespace jit::irgen;

namespace {
std::array<Layout*, Layout::kMaxIndex + 1> s_layoutTable;
}

Layout::Layout(const std::string& description,
               const LayoutFunctions* vtable)
    : m_description(description), m_vtable(vtable) {
  static std::atomic<uint64_t> s_layoutTableIndex;
  m_index = s_layoutTableIndex++;
  always_assert(m_index < kMaxIndex);
  s_layoutTable[m_index] = this;
}

const Layout* layoutForIndex(uint16_t index) {
  auto const layout = s_layoutTable[index];
  assertx(layout->index() == index);
  return layout;
}

SSATmp* Layout::emitSet(IRGS& env, SSATmp* base, SSATmp* key, SSATmp* val) const {
  PUNT(unimpl_bespoke_emitSet);
}

SSATmp* Layout::emitAppend(IRGS& env, SSATmp* base, SSATmp* val) const {
  PUNT(unimpl_bespoke_emitAppend);
}

SSATmp* Layout::emitGet(IRGS& env, SSATmp* base, SSATmp* key, Block* taken) const {
  PUNT(unimpl_bespoke_emitGet);
}

SSATmp* Layout::emitIsset(IRGS& env, SSATmp* base, SSATmp* key) const {
  PUNT(unimpl_bespoke_emitIsset);
}

}}
