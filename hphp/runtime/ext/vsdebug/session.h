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

#ifndef incl_HPHP_VSDEBUG_SESSION_H_
#define incl_HPHP_VSDEBUG_SESSION_H_

#include "hphp/runtime/ext/vsdebug/logging.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/transport.h"
#include "hphp/runtime/ext/vsdebug/command_queue.h"
#include "hphp/util/async-func.h"

namespace HPHP {
namespace VSDEBUG {

struct Debugger;

// This object represents a connected session with a single debugger client.
// It contains any data specific to the connected client's state.
struct DebuggerSession final {
  DebuggerSession(Debugger* debugger);
  virtual ~DebuggerSession();

  void startDummyRequest(const std::string& startupDoc);

  void enqueueDummyCommand(VSCommand* command);

private:

  Debugger* const m_debugger;

  // The "dummy" request thread is a hidden request that provides an execution
  // context from which to execute any debugger command that is not directed at
  // a specific real request. For example: servicing evaluate commands while
  // the debugger is not broken in.

  void runDummy();
  void invokeDummyStartupDocument();

  CommandQueue m_dummyCommandQueue;
  AsyncFunc<DebuggerSession> m_dummyThread;
  std::string m_dummyStartupDoc;
};

}
}

#endif // incl_HPHP_VSDEBUG_SESSION_H_
