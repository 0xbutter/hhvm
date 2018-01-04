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

#ifndef incl_HPHP_VSDEBUG_DEBUGGER_H_
#define incl_HPHP_VSDEBUG_DEBUGGER_H_

#include <atomic>
#include <unordered_map>

#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/ext/vsdebug/logging.h"
#include "hphp/runtime/ext/vsdebug/transport.h"
#include "hphp/runtime/ext/vsdebug/session.h"
#include "hphp/runtime/ext/vsdebug/command_queue.h"
#include "hphp/runtime/ext/vsdebug/hook.h"

namespace HPHP {
namespace VSDEBUG {

// Forward declaration for transport.
struct DebugTransport;

// Forward declaration of debugger session.
struct DebuggerSession;

enum ProgramState {
  LoaderBreakpoint,
  Paused,
  Running
};

// Structure to represent the state of a single request.
struct RequestInfo {
  struct {
    bool hookAttached;
    bool memoryLimitRemoved;
    bool initialBreakpointsSynced;
  } m_flags;
  CommandQueue m_commandQueue;
};

struct Debugger final {
  Debugger() {}
  virtual ~Debugger() {
    shutdown();
  }

  // Sets the transport mechanism to be used to communicate with a debug client.
  // This call transfers ownership of transport from the caller to this object,
  // which will be responsbile for cleaning it up and deleting it before the
  // debugger is destroyed.
  void setTransport(DebugTransport* transport);

  // Invoked by the transport to indicate if a debugger client is currently
  // connected or not. Many debugger events will be skipped if no client is
  // connected to avoid impacting perf when there is no debugger client
  // attached.
  void setClientConnected(bool connected);

  // Shuts down the debugger session and cleans up any resources. This will also
  // unblock any requests that are broken in to the debugger.
  void shutdown();

  // Returns true if there is a client connected.
  inline bool clientConnected() const {
    return m_clientConnected.load(std::memory_order_acquire);
  }

  void sendUserMessage(
    const char* message,
    const char* level = DebugTransport::OutputLevelLog
  );

  // Handle requests.
  void requestInit();
  void requestShutdown();

  // Returns a pointer to the RequestInfo for the current thread.
  RequestInfo* getRequestInfo();

  // Called by the debugger transport when a new message is received from
  // a connected debugger client.
  void onClientMessage(folly::dynamic& message);

private:

  static void cleanupRequestInfo(ThreadInfo* ti, RequestInfo* ri);
  RequestInfo* attachToRequest(ThreadInfo* ti);

  Mutex m_lock;
  DebugTransport* m_transport {nullptr};

  // The DebuggerSession represents the connected debugger client. This object
  // is nullptr if no client is connected, and never nullptr if a client is
  // connected.
  DebuggerSession* m_session {nullptr};

  // The following flag will indicate if there is any debugger client
  // connected to the extension. This allows us to return quickly from
  // debugger hook operations when the debugger is "enabled" but not
  // actually in used due to no connected debugger clients.
  std::atomic<bool> m_clientConnected {false};

  // State of the program.
  ProgramState m_state {ProgramState::Running};

  // Information about all the requests that the debugger is aware of.
  std::unordered_map<ThreadInfo*, RequestInfo*> m_requests;

  // Keeps track of the number of requests that are currently blocked inside
  // the debugger extension due to being paused for any reason (breakpoint,
  // exception, loader break, etc...)
  uint64_t m_pausedRequestCount {0};

  // Keeps track of the total number of requests attached to by this extension
  // since the server was started.
  std::atomic<uint64_t> m_totalRequestCount {0};

  // Indicates which request is the "active" request for debugger client
  // commands, or nullptr if there is no such request.
  ThreadInfo* m_activeRequest {nullptr};
};

}
}

#endif // incl_HPHP_VSDEBUG_DEBUGGER_H_
