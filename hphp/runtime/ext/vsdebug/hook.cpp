/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/vsdebug/hook.h"

namespace HPHP {
namespace VSDEBUG {

struct BreakContext {
  BreakContext(bool breakNoStepOnly)
    : m_debugger(VSDebugExtension::getDebugger()),
      m_requestInfo(m_debugger->getRequestInfo()),
      m_breakNoStepOnly(breakNoStepOnly) {

    if (m_debugger == nullptr || m_requestInfo == nullptr) {
      return;
    }

    VSDebugHook::tryEnterDebugger(m_debugger, m_requestInfo, m_breakNoStepOnly);
  }

  Debugger* m_debugger;
  RequestInfo* m_requestInfo;

  // If true, tryEnterDebugger should try to break only when a step operation
  // is not in progress, otherwise it should always ask the debugger if we
  // should break.
  const bool m_breakNoStepOnly;
};

void VSDebugHook::onRequestInit() {
  BreakContext breakContext(false);
}

void VSDebugHook::onRequestShutdown() {
}

void VSDebugHook::onOpcode(PC pc) {
  RID().setDebuggerIntr(false);
  BreakContext breakContext(true);
}

void VSDebugHook::onFuncEntryBreak(const Func* func) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onFuncExitBreak(const Func* func) {
  // TODO: (Ericblue) NOT IMPLEMENTED
}

void VSDebugHook::onLineBreak(const Unit* unit, int line) {
  BreakContext breakContext(true);

  if (breakContext.m_debugger != nullptr &&
      breakContext.m_requestInfo != nullptr) {

    breakContext.m_debugger->onLineBreakpointHit(
      breakContext.m_requestInfo,
      unit,
      line
    );
  }
}

void VSDebugHook::onExceptionThrown(ObjectData* exception) {
  BreakContext breakContext(true);

  const StringData* name = exception->getVMClass()->name();
  const Variant msg = exception->o_invoke(s_getMsg, init_null(), false);
  const HPHP::String msg_str = msg.isNull() ? empty_string() : msg.toString();

  if (breakContext.m_debugger != nullptr &&
      breakContext.m_requestInfo != nullptr) {

    breakContext.m_debugger->onExceptionBreakpointHit(
      breakContext.m_requestInfo,
      name == nullptr
        ? std::string("Unknown Exception")
        : name->toCppString(),
      msg_str.toCppString()
    );
  }
}

void VSDebugHook::onError(
  const ExtendedException& extendedException,
  int errnum,
  const std::string& message
) {
  BreakContext breakContext(true);

  if (breakContext.m_debugger != nullptr &&
      breakContext.m_requestInfo != nullptr) {

    breakContext.m_debugger->onError(
      breakContext.m_requestInfo,
      extendedException,
      errnum,
      message
    );
  }
}

void VSDebugHook::onStepInBreak(const Unit* unit, int line) {
  BreakContext breakContext(false);
}

void VSDebugHook::onStepOutBreak(const Unit* unit, int line) {
  BreakContext breakContext(false);
}

void VSDebugHook::onNextBreak(const Unit* unit, int line) {
  BreakContext breakContext(true);

  if (breakContext.m_debugger == nullptr ||
      breakContext.m_requestInfo == nullptr) {
    return;
  }

  // When stepping over a multiline statement like a function call,
  // we might be filtering a particular line to avoid hitting it multiple times.
  auto ri = breakContext.m_requestInfo;
  while (!ri->m_nextFilterInfo.empty()) {
    auto currentLocation = Debugger::getVmLocation();
    const Unit* unit = currentLocation.first;
    const HPHP::SourceLoc& loc = currentLocation.second;

    const StepNextFilterInfo& filter = ri->m_nextFilterInfo.back();
    int skipLine0 = filter.skipLine0;
    int skipLine1 = filter.skipLine1;

    if (unit != filter.stepStartUnit ||
        loc.line0 < skipLine0 ||
        loc.line0 > skipLine1) {

      // We stepped into another unit or out of the source range that the
      // statement we're avoiding covers. Pop off the filter.
      ri->m_nextFilterInfo.pop_back();

      // Check the next filter in case we're inside a multi-line statement
      // that is inside a multi-line statement...
      continue;
    } else if (loc.line0 == skipLine0 || loc.line1 == skipLine1) {
      // If this location is filtered out, skip stopping at this PC.
      // Re-enable interrupts so we get invoked again on the next opcode.
      RID().setDebuggerIntr(true);
      phpDebuggerNext();
      return;
    } else {
      // Still in the range of the current filter, but we didn't decide
      // to skip this line. Enter the debugger.
      break;
    }
  }

  VSDebugHook::tryEnterDebugger(
    breakContext.m_debugger,
    breakContext.m_requestInfo,
    false
  );
}

void VSDebugHook::onFileLoad(Unit* efile) {
  BreakContext breakContext(true);

  // Resolve any unresolved breakpoints that may be in this compilation unit.
  if (breakContext.m_debugger != nullptr &&
      breakContext.m_requestInfo != nullptr) {

    breakContext.m_debugger->onCompilationUnitLoaded(
      breakContext.m_requestInfo,
      efile
    );

    tryEnterDebugger(
      breakContext.m_debugger,
      breakContext.m_requestInfo,
      true
    );
  }
}

void VSDebugHook::onDefClass(const Class* cls) {
}

void VSDebugHook::onDefFunc(const Func* func) {
  // TODO: (Ericblue) This routine is not needed unless we add support for
  // function entry breakpoints.
}

void VSDebugHook::tryEnterDebugger(
  Debugger* debugger,
  RequestInfo* requestInfo,
  bool breakNoStepOnly
) {
  if (requestInfo->m_flags.doNotBreak) {
    return;
  }

  // The first time this request enters the debugger, remove the artificial
  // memory limit since a debugger is attached.
  if (!requestInfo->m_flags.memoryLimitRemoved) {
    IniSetting::SetUser(s_memoryLimit, std::numeric_limits<int64_t>::max());
    requestInfo->m_flags.memoryLimitRemoved = true;
  }

  if (!breakNoStepOnly || !Debugger::isStepInProgress(requestInfo)) {
    debugger->enterDebuggerIfPaused(requestInfo);
  }

  // Install any breakpoints that have not yet been set on this request.
  debugger->tryInstallBreakpoints(requestInfo);
}

const StaticString VSDebugHook::s_memoryLimit {"memory_limit"};
const StaticString VSDebugHook::s_getMsg {"getMessage"};

}
}
