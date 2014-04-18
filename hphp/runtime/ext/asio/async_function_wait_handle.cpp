/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/asio/async_function_wait_handle.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/ext/asio/asio_context.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void delete_AsyncFunctionWaitHandle(ObjectData* od, const Class*) {
  auto const waitHandle = static_cast<c_AsyncFunctionWaitHandle*>(od);
  auto const size = waitHandle->resumable()->size();
  auto const base = (char*)(waitHandle + 1) - size;
  waitHandle->~c_AsyncFunctionWaitHandle();
  MM().objFreeLogged(base, size);
}

///////////////////////////////////////////////////////////////////////////////

// max depth of async function stack
const uint16_t MAX_DEPTH = 512;

c_AsyncFunctionWaitHandle::c_AsyncFunctionWaitHandle(Class* cb)
    : c_BlockableWaitHandle(cb), m_child(nullptr), m_privData(),
      m_depth(0) {
}

c_AsyncFunctionWaitHandle::~c_AsyncFunctionWaitHandle() {
  if (LIKELY(isFinished())) {
    assert(!m_child);
    return;
  }

  frame_free_locals_inl_no_hook<false>(actRec(), actRec()->func()->numLocals());
  if (m_child) {
    decRefObj(m_child);
  }
}

void c_AsyncFunctionWaitHandle::t___construct() {
  Object e(SystemLib::AllocInvalidOperationExceptionObject(
        "Define an async functions instead of using this constructor"));
  throw e;
}

void c_AsyncFunctionWaitHandle::ti_setoncreatecallback(const Variant& callback) {
  if (!callback.isNull() &&
      (!callback.isObject() ||
       !callback.getObjectData()->instanceof(c_Closure::classof()))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set AsyncFunctionWaitHandle::onStart: on_start_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnAsyncFunctionCreateCallback(callback.getObjectDataOrNull());
}

void c_AsyncFunctionWaitHandle::ti_setonawaitcallback(const Variant& callback) {
  if (!callback.isNull() &&
      (!callback.isObject() ||
       !callback.getObjectData()->instanceof(c_Closure::classof()))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set AsyncFunctionWaitHandle::onAwait: on_await_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnAsyncFunctionAwaitCallback(callback.getObjectDataOrNull());
}

void c_AsyncFunctionWaitHandle::ti_setonsuccesscallback(const Variant& callback) {
  if (!callback.isNull() &&
      (!callback.isObject() ||
       !callback.getObjectData()->instanceof(c_Closure::classof()))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set AsyncFunctionWaitHandle::onSuccess: on_success_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnAsyncFunctionSuccessCallback(callback.getObjectDataOrNull());
}

void c_AsyncFunctionWaitHandle::ti_setonfailcallback(const Variant& callback) {
  if (!callback.isNull() &&
      (!callback.isObject() ||
       !callback.getObjectData()->instanceof(c_Closure::classof()))) {
    Object e(SystemLib::AllocInvalidArgumentExceptionObject(
      "Unable to set AsyncFunctionWaitHandle::onFail: on_fail_cb not a closure"));
    throw e;
  }
  AsioSession::Get()->setOnAsyncFunctionFailCallback(callback.getObjectDataOrNull());
}

Object c_AsyncFunctionWaitHandle::t_getprivdata() {
  return m_privData;
}

void c_AsyncFunctionWaitHandle::t_setprivdata(const Object& data) {
  m_privData = data;
}

namespace {

const StaticString s__closure_("{closure}");

void checkCreateErrors(c_WaitableWaitHandle* child) {
  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->getCurrentWaitHandleDepth() >= MAX_DEPTH)) {
    Object e(SystemLib::AllocInvalidOperationExceptionObject(
          "Asio stack overflow"));
    throw e;
  }

  if (session->isInContext()) {
    child->enterContext(session->getCurrentContextIdx());
  }
}

}

ObjectData*
c_AsyncFunctionWaitHandle::Create(const ActRec* fp,
                                  Offset offset,
                                  ObjectData* child) {
  assert(fp);
  assert(!fp->resumed());
  assert(fp->func()->isAsync());
  assert(child);
  assert(child->instanceof(c_WaitableWaitHandle::classof()));

  auto child_wh = static_cast<c_WaitableWaitHandle*>(child);
  assert(!child_wh->isFinished());

  checkCreateErrors(child_wh);

  void* obj = Resumable::Create(fp, offset, sizeof(c_AsyncFunctionWaitHandle));
  auto const waitHandle = new (obj) c_AsyncFunctionWaitHandle();
  waitHandle->incRefCount();
  waitHandle->setNoDestruct();
  waitHandle->initialize(child_wh);
  return waitHandle;
}

void c_AsyncFunctionWaitHandle::initialize(c_WaitableWaitHandle* child) {
  auto session = AsioSession::Get();

  m_child = child;
  m_privData = nullptr;
  m_depth = session->getCurrentWaitHandleDepth() + 1;
  blockOn(child);

  // needs to be called with non-zero refcnt
  if (UNLIKELY(session->hasOnAsyncFunctionCreateCallback())) {
    session->onAsyncFunctionCreate(this, child);
  }
}

void c_AsyncFunctionWaitHandle::run() {
  // may happen if scheduled in multiple contexts
  if (getState() != STATE_SCHEDULED) {
    return;
  }

  try {
    setState(STATE_RUNNING);

    // resume async function
    if (LIKELY(m_child->isSucceeded())) {
      // child succeeded, pass the result to the async function
      g_context->resumeAsyncFunc(resumable(), m_child, m_child->getResult());
    } else if (m_child->isFailed()) {
      // child failed, raise the exception inside the async function
      g_context->resumeAsyncFuncThrow(resumable(), m_child,
                                      m_child->getException());
    } else {
      throw FatalErrorException(
          "Invariant violation: child neither succeeded nor failed");
    }

  retry:
    // async function reached RetC, which already set m_resultOrException
    if (isSucceeded()) {
      m_child = nullptr;
      markAsSucceeded();
      return;
    }

    // async function reached AsyncSuspend, which already set m_child
    assert(!m_child->isFinished());
    assert(m_child->instanceof(c_WaitableWaitHandle::classof()));

    // import child into the current context, detect cross-context cycles
    try {
      child()->enterContext(getContextIdx());
    } catch (Object& e) {
      g_context->resumeAsyncFuncThrow(resumable(), m_child, e.get());
      goto retry;
    }

    // detect cycles
    if (UNLIKELY(isDescendantOf(child()))) {
      Object e(createCycleException(child()));
      g_context->resumeAsyncFuncThrow(resumable(), m_child, e.get());
      goto retry;
    }

    // on await callback
    AsioSession* session = AsioSession::Get();
    if (UNLIKELY(session->hasOnAsyncFunctionAwaitCallback())) {
      session->onAsyncFunctionAwait(this, m_child);
    }

    // set up dependency
    blockOn(child());
  } catch (const Object& exception) {
    // process exception thrown by the async function
    m_child = nullptr;
    markAsFailed(exception);
  } catch (...) {
    // process C++ exception
    m_child = nullptr;
    markAsFailed(AsioSession::Get()->getAbruptInterruptException());
    throw;
  }
}

void c_AsyncFunctionWaitHandle::onUnblocked() {
  setState(STATE_SCHEDULED);
  if (isInContext()) {
    getContext()->schedule(this);
  }
}

void c_AsyncFunctionWaitHandle::markAsSucceeded() {
  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->hasOnAsyncFunctionSuccessCallback())) {
    session->onAsyncFunctionSuccess(this, cellAsCVarRef(m_resultOrException));
  }

  done();
}

void c_AsyncFunctionWaitHandle::markAsFailed(const Object& exception) {
  AsioSession* session = AsioSession::Get();
  if (UNLIKELY(session->hasOnAsyncFunctionFailCallback())) {
    session->onAsyncFunctionFail(this, exception);
  }

  setException(exception.get());
}

void
c_AsyncFunctionWaitHandle::suspend(c_WaitableWaitHandle* child, Offset offset) {
  m_child = child;
  resumable()->setOffset(offset);
}

String c_AsyncFunctionWaitHandle::getName() {
  switch (getState()) {
    case STATE_BLOCKED:
    case STATE_SCHEDULED:
    case STATE_RUNNING: {
      String funcName;
      if (actRec()->func()->isClosureBody()) {
        // Can we do better than this?
        funcName = s__closure_;
      } else {
        funcName = actRec()->func()->name()->data();
      }

      String clsName;
      if (actRec()->hasThis()) {
        clsName = actRec()->getThis()->getVMClass()->name()->data();
      } else if (actRec()->hasClass()) {
        clsName = actRec()->getClass()->name()->data();
      } else {
        return funcName;
      }

      return concat3(clsName, "::", funcName);
    }

    default:
      throw FatalErrorException(
          "Invariant violation: encountered unexpected state");
  }
}

c_WaitableWaitHandle* c_AsyncFunctionWaitHandle::getChild() {
  if (getState() == STATE_BLOCKED) {
    assert(m_child);
    return child();
  } else {
    assert(getState() == STATE_SCHEDULED || getState() == STATE_RUNNING);
    return nullptr;
  }
}

void c_AsyncFunctionWaitHandle::enterContextImpl(context_idx_t ctx_idx) {
  switch (getState()) {
    case STATE_BLOCKED:
      // enter child into new context recursively
      assert(m_child);
      child()->enterContext(ctx_idx);
      setContextIdx(ctx_idx);
      break;

    case STATE_SCHEDULED:
      // reschedule so that we get run
      setContextIdx(ctx_idx);
      getContext()->schedule(this);
      break;

    case STATE_RUNNING: {
      Object e(SystemLib::AllocInvalidOperationExceptionObject(
          "Detected cross-context dependency cycle. You are trying to depend "
          "on something that is running you serially."));
      throw e;
    }

    default:
      assert(false);
  }
}

void c_AsyncFunctionWaitHandle::exitContext(context_idx_t ctx_idx) {
  assert(AsioSession::Get()->getContext(ctx_idx));

  // stop before corrupting unioned data
  if (isFinished()) {
    return;
  }

  // not in a context being exited
  assert(getContextIdx() <= ctx_idx);
  if (getContextIdx() != ctx_idx) {
    return;
  }

  switch (getState()) {
    case STATE_BLOCKED:
      // we were already ran due to duplicit scheduling; the context will be
      // updated thru exitContext() call on the non-blocked wait handle we
      // recursively depend on
      break;

    case STATE_SCHEDULED:
      // move us to the parent context
      setContextIdx(getContextIdx() - 1);

      // reschedule if still in a context
      if (isInContext()) {
        getContext()->schedule(this);
      }

      // recursively move all wait handles blocked by us
      for (auto pwh = getFirstParent(); pwh; pwh = pwh->getNextParent()) {
        pwh->exitContextBlocked(ctx_idx);
      }

      break;

    default:
      assert(false);
  }
}

// Get the filename in which execution will proceed when execution resumes.
String c_AsyncFunctionWaitHandle::getFileName() {
  if (actRec()->func()->originalFilename()) {
    return actRec()->func()->originalFilename()->data();
  } else {
    return actRec()->func()->unit()->filepath()->data();
  }
}

// Get the next execution offset
Offset c_AsyncFunctionWaitHandle::getNextExecutionOffset() {
  if (isFinished()) {
    return InvalidAbsoluteOffset;
  }

  always_assert(!isRunning());
  return resumable()->offset();
}

// Get the line number on which execution will proceed when execution resumes.
int c_AsyncFunctionWaitHandle::getLineNumber() {
  if (isFinished()) {
    return -1;
  }

  always_assert(!isRunning());
  return actRec()->func()->unit()->getLineNumber(resumable()->offset());
}

///////////////////////////////////////////////////////////////////////////////
}
