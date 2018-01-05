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

#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/util/process.h"

#include <boost/filesystem.hpp>

namespace HPHP {
namespace VSDEBUG {

Debugger::Debugger() {
}

void Debugger::setTransport(DebugTransport* transport) {
  assert(m_transport == nullptr);
  m_transport = transport;
  setClientConnected(m_transport->clientConnected());
}

void Debugger::setClientConnected(bool connected) {
  DebuggerSession* sessionToDelete = nullptr;
  SCOPE_EXIT {
    if (sessionToDelete != nullptr) {
      delete sessionToDelete;
    }
  };

  {
    Lock lock(m_lock);

    // Store connected first. New request threads will first check this value
    // to quickly determine if a debugger client is connected to avoid having
    // to grab a lock on the request init path in the case where this extension
    // is enabled, but not in use by any client.
    m_clientConnected.store(connected, std::memory_order_release);

    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "Debugger client connected: %s",
      connected ? "YES" : "NO"
    );

    // Defer cleaning up the session until after we've dropped the lock.
    // Shutting down the dummy request thread will cause the worker to call
    // back into this extension since the HHVM context will call requestShutdown
    // on the dummy request.
    if (m_session != nullptr) {
      sessionToDelete = m_session;
      m_session = nullptr;
    }

    if (connected) {
      // Create a new debugger session.
      assert(m_session == nullptr);
      m_session = new DebuggerSession(this);
      if (m_session == nullptr) {
        VSDebugLogger::Log(
          VSDebugLogger::LogLevelError,
          "Failed to allocate debugger session!"
        );
        m_clientConnected.store(false, std::memory_order_release);
      }

      // When the client connects, break the entire program to get it into a
      // known state that matches the thread list being presented in the
      // debugger. Once all threads are wrangled and the front-end is updated,
      // the program can resume execution.
      m_state = ProgramState::LoaderBreakpoint;

      // Attach the debugger to any request threads that were already
      // running before the client connected. We did not attach to them if they
      // were initialized when the client was disconnected to avoid taking a
      // perf hit for a debugger hook that wasn't going to be used.
      //
      // Only do this after seeing at least 1 request via the requestInit path:
      // on initial startup in script mode, the script request thread will have
      // been created already in HHVM main, but is not ready for us to attach
      // yet because extensions are still being initialized.
      if (m_totalRequestCount.load() > 0) {
        ThreadInfo::ExecutePerThread([this] (ThreadInfo* ti) {
          this->attachToRequest(ti);
        });
      }

      // If the script startup thread is waiting for a client connection, wake
      // it up now.
      {
        std::unique_lock<std::mutex> lock(m_connectionNotifyLock);
        m_connectionNotifyCondition.notify_all();
      }
    } else {

      // The client has detached. Walk through any requests we are currently
      // attached to and release them if they are blocked in the debugger.
      for (auto it = m_requests.begin(); it != m_requests.end(); it++) {
        // Clear any undelivered client messages in the request's command queue,
        // since they apply to the debugger session that just ended.
        // NOTE: The request's RequestInfo will be cleaned up and freed when the
        // request completes in requestShutdown. It is not safe to do that from
        // this thread.
        it->second->m_commandQueue.clearPendingMessages();

        // Clear the breakpoint info.
        if (it->second->m_breakpointInfo != nullptr) {
          delete it->second->m_breakpointInfo;
          it->second->m_breakpointInfo = nullptr;
        }

        it->second->m_breakpointInfo = new RequestBreakpointInfo();
        assert(it->second->m_breakpointInfo->m_pendingBreakpoints.empty());
        assert(it->second->m_breakpointInfo->m_unresolvedBreakpoints.empty());
      }

      m_clientInitialized = false;

      // If we launched this request in debugger launch mode, detach of
      // the client should terminate the request.
      if (VSDebugExtension::s_launchMode) {
        interruptAllThreads();
      }

      resumeTarget();
    }
  }
}

void Debugger::setClientInitialized() {
  Lock lock(m_lock);
  if (m_clientInitialized) {
    return;
  }

  m_clientInitialized = true;

  // Send a thread start event for any thread that exists already at the point
  // the debugger client initializes communication so that they appear in the
  // client side thread list.
  for (auto it = m_requestIdMap.begin(); it != m_requestIdMap.end(); it++) {
    if (it->second->m_executing == ThreadInfo::Executing::UserFunctions ||
        it->second->m_executing == ThreadInfo::Executing::RuntimeFunctions) {
      sendThreadEventMessage(it->first, ThreadEventType::ThreadStarted);
    }
  }
}

int Debugger::getCurrentThreadId() {
  Lock lock(m_lock);

  ThreadInfo* const threadInfo = &TI();
  const auto it = m_requestInfoMap.find(threadInfo);
  if (it == m_requestInfoMap.end()) {
    return -1;
  }

  return it->second;
}

void Debugger::cleanupRequestInfo(ThreadInfo* ti, RequestInfo* ri) {
  if (ri->m_flags.hookAttached) {
    DebuggerHook::detach(ti);
  }

  // Shut down the request's command queue. This releases the thread if
  // it is waiting inside the queue.
  ri->m_commandQueue.shutdown();

  if (ri->m_breakpointInfo != nullptr) {
    delete ri->m_breakpointInfo;
  }

  assert(ri->m_serverObjects.size() == 0);
  delete ri;
}

void Debugger::cleanupServerObjectsForRequest(RequestInfo* ri) {
  m_lock.assertOwnedBySelf();

  std::unordered_map<unsigned int, ServerObject*>& objs = ri->m_serverObjects;

  for (auto it = objs.begin(); it != objs.end();) {
    unsigned int objectId = it->first;

    if (m_session != nullptr) {
      m_session->onServerObjectDestroyed(objectId);
    }

    // Free the object. Note if the object is a variable in request memory,
    // the destruction of ServerObject releases the GC root we're holding.
    ServerObject* object = it->second;
    delete object;

    it = objs.erase(it);
  }

  assert(objs.size() == 0);
}

void Debugger::shutdown() {
  if (m_transport == nullptr) {
    return;
  }

  trySendTerminatedEvent();

  m_transport->shutdown();
  setClientConnected(false);

  // m_session is deleted and set to nullptr by setClientConnected(false).
  assert(m_session == nullptr);

  delete m_transport;
  m_transport = nullptr;
}

void Debugger::trySendTerminatedEvent() {
  Lock lock(m_lock);

  folly::dynamic event = folly::dynamic::object;
  sendEventMessage(event, "terminated");
}

void Debugger::sendStoppedEvent(
  const char* reason,
  int64_t threadId
) {
  Lock lock(m_lock);

  folly::dynamic event = folly::dynamic::object;
  event["allThreadsStopped"] = m_pausedRequestCount == m_requests.size();

  if (reason != nullptr) {
    event["reason"] = reason;
    event["description"] = reason;
  }

  if (threadId > 0) {
    event["threadId"] = threadId;
  }

  sendEventMessage(event, "stopped");
}

void Debugger::sendContinuedEvent(int64_t threadId) {
  Lock lock(m_lock);
  folly::dynamic event = folly::dynamic::object;
  event["allThreadsContinued"] = m_pausedRequestCount == 0;
  if (threadId >= 0) {
    event["threadId"] = threadId;
  }

  sendEventMessage(event, "continued");
}

void Debugger::sendUserMessage(const char* message, const char* level) {
  Lock lock(m_lock);

  if (!clientConnected()) {
    return;
  }

  if (m_transport != nullptr) {
    m_transport->enqueueOutgoingUserMessage(message, level);
  }
}

void Debugger::sendEventMessage(folly::dynamic& event, const char* eventType) {
  Lock lock(m_lock);
  if (m_transport != nullptr) {
    m_transport->enqueueOutgoingEventMessage(event, eventType);
  }
}

void Debugger::sendThreadEventMessage(
  int64_t threadId,
  ThreadEventType eventType
) {
  Lock lock(m_lock);

  if (!m_clientInitialized) {
    // Don't start sending the client "thread started" and "thread exited"
    // events before its finished its initialization flow.
    // In this case, we'll send a thread started event for any threads that are
    // currently running when initialization completes (and any threads that
    // exit before that point, the debugger never needs to know about).
    return;
  }

  folly::dynamic event = folly::dynamic::object;

  event["reason"] = eventType == ThreadEventType::ThreadStarted
    ? "started"
    : "exited";

  // TODO: Thread IDs can be 64 bit here, but the VS protocol is going to
  // interpret this as a JavaScript number which has a smaller max value...
  event["threadId"] = threadId;

  sendEventMessage(event, "thread");
}

void Debugger::requestInit() {
  m_totalRequestCount++;

  if (!clientConnected()) {
    // Don't pay for attaching to the thread if no debugger client is connected.
    return;
  }

  ThreadInfo* const threadInfo = &TI();
  bool pauseRequest;
  RequestInfo* requestInfo;

  {
    Lock lock(m_lock);
    bool dummy = (int64_t)Process::GetThreadId() == m_dummyThreadId;

    // In server mode, attach logging hooks to redirect stdout and stderr
    // to the debugger client. This is not needed in launch mode, because
    // the wrapper has the actual stdout and stderr pipes to use directly.
    if (RuntimeOption::ServerExecutionMode()) {
      g_context->setStdout(&m_stdoutHook);

      if (dummy) {
        // Attach to stderr in server mode only for the dummy thread (to show
        // any error spew from evals, etc). Attaching to all requests produces
        // way too much error spew for the client. Users see stderr output
        // for the webserver via server logs.
        Logger::SetThreadHook(&m_stderrHook);
      }
    }

    // Don't attach to the dummy request thread. DebuggerSession manages the
    // dummy requests debugger hook state.
    if (!dummy) {
      requestInfo = attachToRequest(threadInfo);
      pauseRequest = m_state != ProgramState::Running;
    } else {
      pauseRequest = false;
    }
  }

  // If the debugger was already paused when this request started, drop the lock
  // and block the request in its command queue until the debugger resumes.
  // Note: if the debugger client issues a resume between the time the lock
  // is dropped above and entering the command queue, there will be a pending
  // Resume command in this queue, which will cause this thread to unblock.
  if (pauseRequest && requestInfo != nullptr) {
    processCommandQueue(getCurrentThreadId(), requestInfo);
  }
}

void Debugger::enterDebuggerIfPaused(RequestInfo* requestInfo) {
  bool pauseRequest = false;

  {
    Lock lock(m_lock);
    pauseRequest = m_state != ProgramState::Running;

    if (!clientConnected() && VSDebugExtension::s_launchMode) {
      // If the debugger client launched this script in launch mode, and
      // has detached while the request is still running, terminate the
      // request by throwing a fatal PHP exception.
      VSDebugLogger::Log(
        VSDebugLogger::LogLevelInfo,
        "Debugger client detached and we launched this script. "
          "Killing request with fatal error."
      );

      raise_fatal_error(
        "Request terminated due to debugger client detaching.",
        null_array,
        false,
        true,
        true
      );
    }
  }

  if (pauseRequest) {
    if (requestInfo->m_stepReason != nullptr) {
      processCommandQueue(
        getCurrentThreadId(),
        requestInfo,
        requestInfo->m_stepReason
      );
    } else {
      processCommandQueue(getCurrentThreadId(), requestInfo);
    }
  }
}

void Debugger::processCommandQueue(
  int threadId,
  RequestInfo* requestInfo,
  const char* reason /* = "pause" */
) {
  {
    Lock lock(m_lock);
    m_pausedRequestCount++;
    requestInfo->m_pauseRecurseCount++;
    requestInfo->m_totalPauseCount++;

    sendStoppedEvent(reason, threadId);

    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "Thread %d pausing",
      threadId
    );
  }

  requestInfo->m_commandQueue.processCommands();

  {
    Lock lock(m_lock);
    requestInfo->m_pauseRecurseCount--;
    m_pausedRequestCount--;
    sendContinuedEvent(threadId);

    // Any server objects stored for the client for this request are invalid
    // as soon as the thread is allowed to step.
    cleanupServerObjectsForRequest(requestInfo);

    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "Thread %d resumed",
      threadId
    );
  }

  m_resumeCondition.notify_all();
}

RequestInfo* Debugger::attachToRequest(ThreadInfo* ti) {
  // Note: the caller of this routine must hold m_lock.
  RequestInfo* requestInfo = nullptr;

  int threadId;
  auto it = m_requests.find(ti);
  if (it == m_requests.end()) {
    // New request. Insert a request info object into our map.
    threadId = ++m_nextThreadId;
    requestInfo = new RequestInfo();
    if (requestInfo == nullptr) {
      // Failed to allocate request info.
      return nullptr;
    }

    requestInfo->m_breakpointInfo = new RequestBreakpointInfo();
    if (requestInfo->m_breakpointInfo == nullptr) {
      // Failed to allocate breakpoint info.
      delete requestInfo;
      return nullptr;
    }

    m_requests.emplace(std::make_pair(ti, requestInfo));
    m_requestIdMap.emplace(std::make_pair(threadId, ti));
    m_requestInfoMap.emplace(std::make_pair(ti, threadId));
  } else {
    requestInfo = it->second;
    auto idIt = m_requestInfoMap.find(ti);
    assert(idIt != m_requestInfoMap.end());
    threadId = idIt->second;
  }

  assert(requestInfo != nullptr && requestInfo->m_breakpointInfo != nullptr);

  if (ti->m_executing == ThreadInfo::Executing::UserFunctions ||
      ti->m_executing == ThreadInfo::Executing::RuntimeFunctions) {
    sendThreadEventMessage(threadId, ThreadEventType::ThreadStarted);
  }


  // Try to attach our debugger hook to the request.
  if (!requestInfo->m_flags.hookAttached) {
    if (DebuggerHook::attach<VSDebugHook>(ti)) {
      ti->m_reqInjectionData.setDebuggerIntr(true);
      ti->m_reqInjectionData.setFlag(DebuggerSignalFlag);

      requestInfo->m_flags.hookAttached = true;

      // Install all breakpoints as pending for this request.
      const std::unordered_set<int> breakpoints =
        m_session->getBreakpointManager()->getAllBreakpointIds();
      for (auto it = breakpoints.begin(); it != breakpoints.end(); it++) {
        requestInfo->m_breakpointInfo->m_pendingBreakpoints.emplace(*it);
      }
    } else {
      m_transport->enqueueOutgoingUserMessage(
        "Failed to attach to new HHVM request: another debugger is already "
          "attached.",
        DebugTransport::OutputLevelError
      );
    }
  }

  return requestInfo;
}

void Debugger::requestShutdown() {
  auto const threadInfo = &TI();
  RequestInfo* requestInfo = nullptr;
  int threadId = -1;

  SCOPE_EXIT {
    if (clientConnected() && threadId >= 0) {
      sendThreadEventMessage(threadId, ThreadEventType::ThreadExited);
    }

    if (requestInfo != nullptr) {
      cleanupServerObjectsForRequest(requestInfo);
      cleanupRequestInfo(threadInfo, requestInfo);
    }
  };

  {
    Lock lock(m_lock);
    auto it = m_requests.find(threadInfo);
    if (it == m_requests.end()) {
      return;
    }

    requestInfo = it->second;
    m_requests.erase(it);

    auto infoItr = m_requestInfoMap.find(threadInfo);
    assert(infoItr != m_requestInfoMap.end());

    threadId = infoItr->second;
    auto idItr = m_requestIdMap.find(threadId);
    assert(idItr != m_requestIdMap.end());

    m_requestIdMap.erase(idItr);
    m_requestInfoMap.erase(infoItr);

    g_context->setStdout(nullptr);
    Logger::SetThreadHook(nullptr);
  }
}

RequestInfo* Debugger::getRequestInfo(int threadId /* = -1 */) {
  Lock lock(m_lock);

  if (threadId != -1) {
    // Find the info for the requested thread ID.
    auto it = m_requestIdMap.find(threadId);
    if (it != m_requestIdMap.end()) {
      auto requestIt = m_requests.find(it->second);
      if (requestIt != m_requests.end()) {
        return requestIt->second;
      }
    }
  } else {
    // Find the request info for the current request thread.
    auto it = m_requests.find(&TI());
    if (it != m_requests.end()) {
      return it->second;
    }
  }

  return nullptr;
}

bool Debugger::executeClientCommand(
  VSCommand* command,
  std::function<bool(DebuggerSession* session,
                     folly::dynamic& responseMsg)> callback
) {
  Lock lock(m_lock);

  // If there is no debugger client connected anymore, the client command
  // should not be processed, and the target request thread should resume.
  if (!clientConnected()) {
    return true;
  }

  try {
    enforceRequiresBreak(command);

    // Invoke the command execute callback. It will return true if this thread
    // should be resumed, or false if it should continue to block in its
    // command queue.
    folly::dynamic responseMsg = folly::dynamic::object;
    bool resumeThread = callback(m_session, responseMsg);

    if (command->commandTarget() != CommandTarget::WorkItem) {
      sendCommandResponse(command, responseMsg);
    }

    return resumeThread;
  } catch (DebuggerCommandException e) {
    reportClientMessageError(command->getMessage(), e.what());
  } catch (...) {
    reportClientMessageError(command->getMessage(), InternalErrorMsg);
  }

  // On error, do not resume the request thread.
  return false;
}

void Debugger::executeWithoutLock(std::function<void()> callback) {
  m_lock.assertOwnedBySelf();
  m_lock.unlock();

  callback();

  m_lock.lock();
  m_lock.assertOwnedBySelf();
}

void Debugger::reportClientMessageError(
  folly::dynamic& clientMsg,
  const char* errorMessage
) {
  try {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Failed to process client message (%s): %s",
      folly::toJson(clientMsg).c_str(),
      errorMessage
    );

    folly::dynamic responseMsg = folly::dynamic::object;
    responseMsg["success"] = false;
    responseMsg["request_seq"] = clientMsg["seq"];
    responseMsg["command"] = clientMsg["command"];
    responseMsg["message"] = errorMessage;

    m_transport->enqueueOutgoingMessageForClient(
      responseMsg,
      DebugTransport::MessageTypeResponse
    );

    // Print an error to the debugger console to inform the user as well.
    sendUserMessage(errorMessage, DebugTransport::OutputLevelError);
  } catch (...) {
    // We tried.
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Unexpected failure while trying to send response to client."
    );
  }
}

void Debugger::sendCommandResponse(
  VSCommand* command,
  folly::dynamic& responseMsg
) {
  folly::dynamic& clientMsg = command->getMessage();
  responseMsg["success"] = true;
  responseMsg["request_seq"] = clientMsg["seq"];
  responseMsg["command"] = clientMsg["command"];

  m_transport->enqueueOutgoingMessageForClient(
    responseMsg,
    DebugTransport::MessageTypeResponse
  );
}

void Debugger::resumeTarget() {
  m_lock.assertOwnedBySelf();

  m_state = ProgramState::Running;

  // Resume every paused request. Each request will send a thread continued
  // event when it exits its command loop.
  for (auto it = m_requests.begin(); it != m_requests.end(); it++) {
    RequestInfo* ri = it->second;
    ri->m_stepReason = nullptr;

    if (ri->m_pauseRecurseCount > 0) {
      VSCommand* resumeCommand = ContinueCommand::createInstance(this);
      ri->m_commandQueue.dispatchCommand(resumeCommand);
    }
  }

  sendContinuedEvent(-1);
}

Debugger::PrepareToPauseResult
Debugger::prepareToPauseTarget(RequestInfo* requestInfo) {
  m_lock.assertOwnedBySelf();

  if (m_state == ProgramState::Paused && isStepInProgress(requestInfo)) {
    // A step operation for a single request is still in the middle of
    // handling whatever caused us to break execution of the other threads
    // in the first place. We don't need to wait for resume here.
    return clientConnected() ? ReadyToPause : ErrorNoClient;
  }

  while (m_state != ProgramState::Running) {
    m_lock.assertOwnedBySelf();

    // If a resume is currently in progress, we must wait for all threads
    // to resume execution before breaking again. Otherwise, the client will
    // see interleaved continue and stop events and the the state of the UX
    // becomes undefined.
    std::unique_lock<std::mutex> conditionLock(m_resumeMutex);
    while (m_pausedRequestCount > 0) {
      // Need to drop m_lock before waiting.
      m_lock.unlock();

      m_resumeCondition.wait(conditionLock);
      conditionLock.unlock();

      // And re-acquire it before continuing.
      m_lock.lock();
      conditionLock.lock();
    }

    // Between the time the resume condition was notified and the time this
    // thread re-acquired m_lock, it is possible another thread paused the
    // target again. If that happens, we need to enter the command queue
    // and service the other pause before we can raise a breakpoint for this
    // request thread.
    m_lock.assertOwnedBySelf();
    if (requestInfo != nullptr && m_state != ProgramState::Running) {
      // Drop the lock and enter the command queue.
      m_lock.unlock();
      processCommandQueue(getCurrentThreadId(), requestInfo);

      // Re-acquire before continuing.
      m_lock.lock();
    }
  }

  m_lock.assertOwnedBySelf();
  assert(requestInfo == nullptr || m_state == ProgramState::Running);

  return clientConnected() ? ReadyToPause : ErrorNoClient;
}

void Debugger::pauseTarget(RequestInfo* ri, const char* stopReason) {
  m_lock.assertOwnedBySelf();

  m_state = ProgramState::Paused;

  sendStoppedEvent(stopReason, getCurrentThreadId());

  if (ri != nullptr) {
    clearStepOperation(ri);
  }

  interruptAllThreads();
}

void Debugger::onClientMessage(folly::dynamic& message) {
  Lock lock(m_lock);

  // It's possible the client disconnected between the time the message was
  // received and when the lock was acquired in this routine. If the client
  // has gone, do not process the message.
  if (!clientConnected()) {
    return;
  }

  VSCommand* command = nullptr;
  SCOPE_EXIT {
    if (command != nullptr) {
      delete command;
    }
  };

  try {

    // All valid client messages should have a sequence number and type.
    try {
      const auto& seq = message["seq"];
      if (!seq.isInt()) {
        throw DebuggerCommandException("Invalid message sequence number.");
      }

      const auto& type = message["type"];
      if (!type.isString() || type.getString().empty()) {
        throw DebuggerCommandException("Invalid command type.");
      }
    } catch (std::out_of_range e) {
      throw DebuggerCommandException(
        "Message is missing a required attribute."
      );
    }

    if (!VSCommand::parseCommand(this, message, &command)) {
      assert(command == nullptr);

      try {
        auto cmdName = message["command"];
        if (cmdName.isString()) {
          std::string commandName = cmdName.asString();
          std::string errorMsg("The command \"");
          errorMsg += commandName;
          errorMsg += "\" was invalid or is not implemented in the debugger.";
          throw DebuggerCommandException(errorMsg.c_str());
        }
      } catch (std::out_of_range e) {
      }

      throw DebuggerCommandException(
        "The command was invalid or is not implemented in the debugger."
      );
    }

    assert(command != nullptr);
    enforceRequiresBreak(command);

    // Otherwise this is a normal command. Dispatch it to its target.
    switch(command->commandTarget()) {
      case CommandTarget::None:
      case CommandTarget::WorkItem:
        if (command->execute()) {
          // The command requested that the target be resumed. A command with
          // CommandTarget == None that does this resumes the entire program.
          resumeTarget();
        }
        break;
      case CommandTarget::Request:
        // Dispatch this command to the correct request.
        {
          const auto threadId = command->targetThreadId(m_session);
          auto it = m_requestIdMap.find(threadId);
          if (it != m_requestIdMap.end()) {
            const auto request = m_requests.find(it->second);
            assert(request != m_requests.end());
            request->second->m_commandQueue.dispatchCommand(command);

            // Lifetime of command is now owned by the request thread's queue.
            command = nullptr;
          } else {
            constexpr char* errorMsg =
              "The requested thread ID does not exist in the target.";
            reportClientMessageError(message, errorMsg);
          }
        }
        break;
      case CommandTarget::Dummy:
        // Dispatch this command to the dummy thread.
        m_session->enqueueDummyCommand(command);

        // Lifetime of command is now owned by the dummy thread's queue.
        command = nullptr;
        break;
      default:
        assert(false);
    }
  } catch (DebuggerCommandException e) {
    reportClientMessageError(message, e.what());
  } catch (...) {
    reportClientMessageError(message, InternalErrorMsg);
  }
}

void Debugger::waitForClientConnection() {
  std::unique_lock<std::mutex> lock(m_connectionNotifyLock);
  if (clientConnected()) {
    return;
  }

  while (!clientConnected()) {
    m_connectionNotifyCondition.wait(lock);
  }
}

void Debugger::setClientPreferences(ClientPreferences& preferences) {
  Lock lock(m_lock);
  if (!clientConnected()) {
    return;
  }

  assert(m_session != nullptr);
  m_session->setClientPreferences(preferences);
}

ClientPreferences Debugger::getClientPreferences() {
  Lock lock(m_lock);
  if (!clientConnected()) {
    ClientPreferences empty = {};
    return empty;
  }

  assert(m_session != nullptr);
  return m_session->getClientPreferences();
}

void Debugger::startDummyRequest(const std::string& startupDoc) {
  Lock lock(m_lock);
  if (!clientConnected()) {
    return;
  }

  assert(m_session != nullptr);
  m_session->startDummyRequest(startupDoc);
}

void Debugger::setDummyThreadId(int64_t threadId) {
  Lock lock(m_lock);
  m_dummyThreadId = threadId;
}

void Debugger::onBreakpointAdded(int bpId) {
  Lock lock(m_lock);

  assert(m_session != nullptr);

  // Now to actually install the breakpoints, each request thread needs to
  // process the bp and set it in some TLS data structures. If the program
  // is already paused, then every request thread is blocking in its command
  // loop: we'll put a work item to resolve the bp in each command queue.
  //
  // Otherwise, if the program is running, we need to gain control of each
  // request thread by interrupting it. It will install the bp when it
  // calls into the command hook on the next Hack opcode.
  for (auto it = m_requests.begin(); it != m_requests.end(); it++) {
    ThreadInfo* ti = it->first;
    RequestInfo* ri = it->second;

    ti->m_reqInjectionData.setDebuggerIntr(true);
    ri->m_breakpointInfo->m_pendingBreakpoints.emplace(bpId);

    if (m_state != ProgramState::Running) {
      const auto cmd = ResolveBreakpointsCommand::createInstance(this);
      ri->m_commandQueue.dispatchCommand(cmd);
    }
  }
}

void Debugger::tryInstallBreakpoints(RequestInfo* ri) {
  Lock lock(m_lock);

  if (!clientConnected()) {
    return;
  }

  // Create a map of the normalized file paths of all compilation units that
  // have already been loaded by this request before the debugger attached to
  // it to allow for quick lookup when resolving breakpoints. Any units loaded
  // after this will be added to the map by onCompilationUnitLoaded().
  if (!ri->m_flags.compilationUnitsMapped) {
    ri->m_flags.compilationUnitsMapped = true;
    const auto evaledFiles = g_context->m_evaledFiles;
    for (auto it = evaledFiles.begin(); it != evaledFiles.end(); it++) {
      const HPHP::Unit* compilationUnit = it->second.unit;
      const std::string filePath = getFilePathForUnit(compilationUnit);
      ri->m_breakpointInfo->m_loadedUnits[filePath] = compilationUnit;
    }
  }

  // For any breakpoints that are pending for this request, try to resolve
  // and install them, or mark them as unresolved.
  BreakpointManager* bpMgr = m_session->getBreakpointManager();
  auto& pendingBps = ri->m_breakpointInfo->m_pendingBreakpoints;
  for (auto it = pendingBps.begin(); it != pendingBps.end();) {
    const int breakpointId = *it;
    const Breakpoint* bp = bpMgr->getBreakpointById(breakpointId);

    // It's ok if bp was not found. The client could have removed the
    // breakpoint before this request got a chance to install it.
    if (bp != nullptr) {
      bool resolved = tryResolveBreakpoint(ri, breakpointId, bp);

      if (!resolved) {
        if (!RuntimeOption::RepoAuthoritative) {
          // It's possible this compilation unit just isn't loaded yet. Try
          // to force a pre-load and compile of the unit and place the bp.
          HPHP::String unitPath(bp->m_path.c_str());
          const auto compilationUnit = lookupUnit(unitPath.get(), "", nullptr);

          if (compilationUnit != nullptr) {
            ri->m_breakpointInfo->m_loadedUnits[bp->m_path] = compilationUnit;
            resolved = tryResolveBreakpoint(ri, breakpointId, bp);
          }

          if (!resolved) {
            std::string resolveMsg = "Warning: request ";
            resolveMsg += std::to_string(getCurrentThreadId());
            resolveMsg += " could not resolve breakpoint #";
            resolveMsg += std::to_string(breakpointId);
            resolveMsg += ". The Hack/PHP file at ";
            resolveMsg += bp->m_path;

            if (compilationUnit == nullptr) {
              resolveMsg += " could not be loaded, or failed to compile.";
            } else {
              resolveMsg += " was loaded, but the breakpoint did not resolve "
                "to any executable instruction.";
            }

            sendUserMessage(
              resolveMsg.c_str(),
              DebugTransport::OutputLevelWarning
            );
          }
        }

        // This breakpoint could not be resolved yet. As new compilation units
        // are loaded, we'll try again.
        if (!resolved) {
          ri->m_breakpointInfo->m_unresolvedBreakpoints.emplace(breakpointId);
        }
      }
    }

    it = pendingBps.erase(it);
  }

  assert(ri->m_breakpointInfo->m_pendingBreakpoints.empty());
}

bool Debugger::tryResolveBreakpoint(
  RequestInfo* ri,
  const int bpId,
  const Breakpoint* bp
) {
  // Search all compilation units loaded by this request for a matching location
  // for this breakpoint.
  const auto& loadedUnits = ri->m_breakpointInfo->m_loadedUnits;
  for (auto it = loadedUnits.begin(); it != loadedUnits.end(); it++) {
    if (tryResolveBreakpointInUnit(ri, bpId, bp, it->first, it->second)) {
      // Found a match, and installed the breakpoint!
      return true;
    }
  }

  return false;
}

bool Debugger::tryResolveBreakpointInUnit(
  const RequestInfo* ri,
  int bpId,
  const Breakpoint* bp,
  const std::string& unitFilePath,
  const HPHP::Unit* compilationUnit
) {
  if (bp->m_path != unitFilePath) {
    return false;
  }

  std::pair<int,int> lines =
    calibrateBreakpointLineInUnit(compilationUnit, bp->m_line);

  if (lines.first > 0 && lines.second != lines.first) {
    lines = calibrateBreakpointLineInUnit(compilationUnit, lines.first);
  }

  if (lines.first < 0) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "NOT installing bp ID %d in file %s. No source locations matching "
        " line %d were found.",
      bpId,
      unitFilePath.c_str(),
      bp->m_line
    );
    return false;
  }

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Installing bp ID %d at line %d (original line was %d) of file %s.",
    bpId,
    lines.first,
    bp->m_line,
    unitFilePath.c_str()
  );

  if (!phpAddBreakPointLine(compilationUnit, lines.first)) {
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelError,
      "Installing %d at line %d of file %s FAILED in phpAddBreakPointLine!",
      bpId,
      lines.first,
      unitFilePath.c_str()
    );
    return false;
  }

  m_session->getBreakpointManager()->onBreakpointResolved(
    bpId,
    lines.first,
    lines.second,
    0,
    0,
    unitFilePath
  );

  return true;
}

std::pair<int, int> Debugger::calibrateBreakpointLineInUnit(
  const Unit* unit,
  int bpLine
) {
  // Attempt to find a matching source location entry in the compilation unit
  // that corresponds to the breakpoint's requested line number. Note that the
  // line provided by the client could be in the middle of a multi-line
  // statement, or could be on a line that contains multiple statements. It
  // could also be in whitespace, or past the end of the file.
  std::pair<int, int> bestLocation = {-1, -1};
  int bestDistance = INT_MAX;

  for (auto const& tableEntry : getSourceLocTable(unit)) {
    const SourceLoc& sourceLocation = tableEntry.val();

    // If this source location ends before the bp's line. No match.
    if (!sourceLocation.valid() || sourceLocation.line1 < bpLine) {
      continue;
    }

    // If we found a single line source location that begins at the bp's line,
    // this is the ideal case, and is where the breakpoint should be placed.
    if (sourceLocation.line0 == sourceLocation.line1 &&
        sourceLocation.line0 == bpLine) {
        bestLocation.first = sourceLocation.line0;
        bestLocation.second = sourceLocation.line1;
        break;
    }

    // Otherwise, choose the source line whose ending line is closest to the
    // breakpoint's desired line.
    int distance = sourceLocation.line1 - bpLine;
    if (distance < bestDistance) {
      bestDistance = distance;
      bestLocation.first = sourceLocation.line0;
      bestLocation.second = sourceLocation.line1;
    }
  }

  return bestLocation;
}

void Debugger::onCompilationUnitLoaded(
  RequestInfo* ri,
  const HPHP::Unit* compilationUnit
) {
  Lock lock(m_lock);

  if (!clientConnected()) {
    return;
  }

  const auto filePath = getFilePathForUnit(compilationUnit);
  ri->m_breakpointInfo->m_loadedUnits[filePath] = compilationUnit;

  // See if any unresolved breakpoints for this request can be placed in the
  // compilation unit that just loaded.
  BreakpointManager* bpMgr = m_session->getBreakpointManager();
  auto& unresolvedBps = ri->m_breakpointInfo->m_unresolvedBreakpoints;
  for (auto it = unresolvedBps.begin(); it != unresolvedBps.end();) {
    const int bpId = *it;
    const Breakpoint* bp = bpMgr->getBreakpointById(bpId);
    if (bp == nullptr ||
        tryResolveBreakpointInUnit(ri, bpId, bp, filePath, compilationUnit)) {

      // If this breakpoint no longer exists (it was removed by the client),
      // or it was successfully installed, then it is no longer unresolved.
      it = unresolvedBps.erase(it);
    } else {
      // Otherwise, move on to the next unresolved breakpoint.
      it++;
    }
  }
}

void Debugger::onLineBreakpointHit(
  RequestInfo* ri,
  const HPHP::Unit* compilationUnit,
  int line
) {

  std::string stopReason;
  int matchingBpId = -1;
  const std::string filePath = getFilePathForUnit(compilationUnit);

  {
    Lock lock(m_lock);

    if (prepareToPauseTarget(ri) != PrepareToPauseResult::ReadyToPause) {
      return;
    }

    BreakpointManager* bpMgr = m_session->getBreakpointManager();
    const auto fileBps = bpMgr->getBreakpointIdsByFile(filePath);

    for (auto it = fileBps.begin(); it != fileBps.end(); it++) {
      const int bpId = *it;
      const Breakpoint* bp = bpMgr->getBreakpointById(bpId);
      if (line == bp->m_resolvedLocation.m_startLine) {
        matchingBpId = bpId;
        stopReason = getStopReasonForBp(matchingBpId, bp->m_path, bp->m_line);

        // Breakpoint hit!
        pauseTarget(ri, stopReason.c_str());
        bpMgr->onBreakpointHit(bpId);
        break;
      }
    }
  }

  if (matchingBpId >= 0) {
    // If an active breakpoint was found at this location, enter the debugger.
    processCommandQueue(getCurrentThreadId(), ri, stopReason.c_str());
  } else if (ri->m_runToLocationInfo.path == filePath &&
             line == ri->m_runToLocationInfo.line) {

    // Hit our run to location destination!
    stopReason = "Run to location";
    ri->m_runToLocationInfo.path = "";
    ri->m_runToLocationInfo.line = -1;

    // phpRemoveBreakpointLine doesn't refcount or anything, so it's only
    // safe to remove this if there is no real bp at the line.
    bool realBp = false;
    BreakpointManager* bpMgr = m_session->getBreakpointManager();
    const auto bpIds = bpMgr->getBreakpointIdsByFile(filePath);
    for (auto it = bpIds.begin(); it != bpIds.end(); it++) {
      Breakpoint* bp = bpMgr->getBreakpointById(*it);
      if (bp->m_line == line) {
        realBp = true;
        break;
      }
    }

    if (!realBp) {
      phpRemoveBreakPointLine(compilationUnit, line);
    }

    pauseTarget(ri, stopReason.c_str());
    processCommandQueue(getCurrentThreadId(), ri, stopReason.c_str());
  } else {
    // This breakpoint no longer exists. Remove it from the VM.
    VSDebugLogger::Log(
      VSDebugLogger::LogLevelInfo,
      "Request hit bp that no longer exists, removing from VM. %s:%d",
      filePath.c_str(),
      line
    );
    phpRemoveBreakPointLine(compilationUnit, line);
  }
}

void Debugger::onExceptionBreakpointHit(
  RequestInfo* ri,
  const std::string& exceptionName,
  const std::string& exceptionMsg
) {
  std::string stopReason("Exception (");
  stopReason += exceptionName;
  stopReason += ") thrown";

  std::string userMsg = "Request ";
  userMsg += std::to_string(getCurrentThreadId());
  userMsg += ": ";
  userMsg += stopReason + ": ";
  userMsg += exceptionMsg;

  {
    Lock lock(m_lock);

    if (!clientConnected()) {
      return;
    }

    BreakpointManager* bpMgr = m_session->getBreakpointManager();
    ExceptionBreakMode breakMode = bpMgr->getExceptionBreakMode();

    switch (breakMode) {
      case BreakNone:
        // Do not break on exceptions.
        return;
      case BreakUnhandled:
      case BreakUserUnhandled:
        // The PHP VM doesn't give us any way to distinguish between handled
        // and unhandled exceptions. Print a message to the console but do
        // not break.
        sendUserMessage(userMsg.c_str(), DebugTransport::OutputLevelWarning);
        return;
      case BreakAll:
        break;
      default:
        assert(false);
    }

    if (prepareToPauseTarget(ri) != PrepareToPauseResult::ReadyToPause) {
      return;
    }

    pauseTarget(ri, stopReason.c_str());
  }

  processCommandQueue(getCurrentThreadId(), ri, stopReason.c_str());
}

void Debugger::onAsyncBreak() {
  Lock lock(m_lock);

  if (m_state == ProgramState::Paused) {
    // Already paused.
    return;
  }

  if (prepareToPauseTarget(nullptr) != PrepareToPauseResult::ReadyToPause) {
    return;
  }

  VSDebugLogger::Log(
    VSDebugLogger::LogLevelInfo,
    "Debugger paused due to async-break request from client."
  );

  pauseTarget(nullptr, "Async-break");
}

void Debugger::onError(
  RequestInfo* requestInfo,
  const ExtendedException& extendedException,
  int errnum,
  const std::string& message
) {
  const char* phpError;
  switch (static_cast<ErrorMode>(errnum)) {
    case ErrorMode::ERROR:
    case ErrorMode::CORE_ERROR:
    case ErrorMode::COMPILE_ERROR:
    case ErrorMode::USER_ERROR:
      phpError = "Fatal error";
      break;
    case ErrorMode::RECOVERABLE_ERROR:
      phpError = "Catchable fatal error";
      break;
    case ErrorMode::WARNING:
    case ErrorMode::CORE_WARNING:
    case ErrorMode::COMPILE_WARNING:
    case ErrorMode::USER_WARNING:
      phpError = "Warning";
      break;
    case ErrorMode::PARSE:
      phpError = "Parse error";
      break;
    case ErrorMode::NOTICE:
    case ErrorMode::USER_NOTICE:
      phpError = "Notice";
      break;
    case ErrorMode::STRICT:
      phpError = "Strict standards";
      break;
    case ErrorMode::PHP_DEPRECATED:
    case ErrorMode::USER_DEPRECATED:
      phpError = "Deprecated";
      break;
    default:
      phpError = "Unknown error";
      break;
  }

  onExceptionBreakpointHit(requestInfo, phpError, message);
}

std::string Debugger::getFilePathForUnit(const HPHP::Unit* compilationUnit) {
  const auto path =
    HPHP::String(const_cast<StringData*>(compilationUnit->filepath()));
  return File::TranslatePath(path).toCppString();
}

std::string Debugger::getStopReasonForBp(
  const int id,
  const std::string& path,
  const int line
) {
  std::string description("Breakpoint " + std::to_string(id));
  if (!path.empty()) {
    const char* name = boost::filesystem::path(path.c_str()).filename().c_str();
    description += " (";
    description += name;
    description += ":";
    description += std::to_string(line);
    description += ")";
  }

  return description;
}

void Debugger::interruptAllThreads() {
  for (auto it = m_requests.begin(); it != m_requests.end(); it++) {
    ThreadInfo* ti = it->first;
    ti->m_reqInjectionData.setDebuggerIntr(true);
  }
}

void DebuggerStdoutHook::operator()(const char* str, int len) {
  std::string output = std::string(str, len);
  m_debugger->sendUserMessage(
    output.c_str(),
    DebugTransport::OutputLevelStdout);
}

void DebuggerStderrHook::operator()(
  const char*,
  const char* msg,
  const char* ending
) {
  m_debugger->sendUserMessage(msg, DebugTransport::OutputLevelStderr);
}

}
}
