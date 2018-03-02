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

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/statement/statement_list.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP {
namespace VSDEBUG {

EvaluateCommand::EvaluateCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message),
    m_frameId{0} {

  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  const int frameId = tryGetInt(args, "frameId", -1);
  m_frameId = frameId;
}

EvaluateCommand::~EvaluateCommand() {
}

FrameObject* EvaluateCommand::getFrameObject(DebuggerSession* session) {
  if (m_frameObj != nullptr) {
    return m_frameObj;
  }

  m_frameObj = session->getFrameObject(m_frameId);
  return m_frameObj;
}

request_id_t EvaluateCommand::targetThreadId(DebuggerSession* session) {
  FrameObject* frame = getFrameObject(session);
  if (frame == nullptr) {
    // Execute the eval in the dummy context.
    return Debugger::kDummyTheadId;
  }

  return frame->m_requestId;
}

ExecutionContext::EvaluationResult evaluate(
  Debugger* debugger,
  RequestInfo* ri,
  HPHP::Unit* unit,
  int frameDepth,
  bool silent
) {
  ExecutionContext::EvaluationResult result;

  if (silent) {
    SilentEvaluationContext silentContext(debugger, ri);
    result = g_context->evalPHPDebugger(unit, frameDepth);
  } else {
    result = g_context->evalPHPDebugger(unit, frameDepth);
  }

  return result;
}

bool EvaluateCommand::executeImpl(
  DebuggerSession* session,
  folly::dynamic* responseMsg
) {
  folly::dynamic& message = getMessage();
  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  const std::string& expression = tryGetString(args, "expression", "");
  const auto threadId = targetThreadId(session);

  std::string evalExpression = expression;
  preparseEvalExpression(&evalExpression);
  if (evalExpression.empty()) {
    throw DebuggerCommandException("No expression provided to evaluate.");
  }

  // Enable bypassCheck, which allows eval statements from the debugger to
  // violate visibility checks on object properties.
  g_context->debuggerSettings.bypassCheck = true;

  // Set the error reporting level to 0 so non-fatal errors in the expression
  // are swallowed.
  RequestInjectionData& rid = RID();
  const int previousErrorLevel = rid.getErrorReportingLevel();
  rid.setErrorReportingLevel(0);

  RequestInfo* ri = m_debugger->getRequestInfo();
  assert(ri->m_evaluateCommandDepth >= 0);
  ri->m_evaluateCommandDepth++;

  // Track if the evaluation command caused any opcode stepping to occur
  // so we know if we need to re-send a stop event after the evaluation.
  int previousPauseCount = ri->m_totalPauseCount;
  bool isDummy = m_debugger->isDummyRequest();

  // Put everything back on scope exit.
  SCOPE_EXIT {
    g_context->debuggerSettings.bypassCheck = false;
    rid.setErrorReportingLevel(previousErrorLevel);

    ri->m_evaluateCommandDepth--;
    assert(ri->m_evaluateCommandDepth >= 0);

    if (ri->m_evaluateCommandDepth == 0 && isDummy) {
      // The dummy request only appears in the client UX while it is
      // stopped at a breakpoint during an evaluation (because the user
      // needs to see a call stack and scopes at that point). Otherwise,
      // existance of the dummy is hiden from the user. If the dummy is
      // no longer executing any evaluation, send a thread exited event
      // to remove it from the front-end UX.
      m_debugger->sendThreadEventMessage(
        0,
        Debugger::ThreadEventType::ThreadExited
      );

      g_context->exitDebuggerDummyEnv();
    }
  };

  Unit* unit = compile_string(evalExpression.c_str(), evalExpression.size());
  if (unit == nullptr) {
    // The compiler will already have printed more detailed error messages
    // to stderr, which is redirected to the debugger client's console.
    throw DebuggerCommandException("Error compiling expression.");
  }

  FrameObject* frameObj = getFrameObject(session);
  int frameDepth = frameObj == nullptr ? 0 : frameObj->m_frameDepth;

  if (ri->m_evaluateCommandDepth == 1 && isDummy) {
    // Set up the dummy evaluation environment unless we have recursively
    // re-entered eval on the dummy thread, in which case it's already set.
    g_context->enterDebuggerDummyEnv();

    // Show the dummy thread while it is doing an evaluation so it can
    // present a call stack if it hits a breakpoint during the eval.
    m_debugger->sendThreadEventMessage(
      0,
      Debugger::ThreadEventType::ThreadStarted
    );
  }

  // We must drop the lock before calling evalPHPDebugger because the eval
  // is permitted to hit breakpoints, which can call back into Debugger and
  // enter a command queue. Threads must never enter the command queue while
  // holding the debugger lock, because we would be unable to processes more
  // commands from the client: there'd be no way to resume the blocked request.
  ExecutionContext::EvaluationResult result;

  // If the client indicates this evaluation is for a watch expression, or
  // hover evaluation, silence all errors.
  const std::string evalContext = tryGetString(args, "context", "");
  bool evalSilent = evalContext == "watch" || evalContext == "hover";
  m_debugger->executeWithoutLock(
    [&]() {
        result = evaluate(m_debugger, ri, unit, frameDepth, evalSilent);
    });

  if (previousPauseCount != ri->m_totalPauseCount &&
      ri->m_pauseRecurseCount > 0) {

    m_debugger->sendStoppedEvent(
      "breakpoint",
      "Evaluation returned",
      threadId
    );
  }

  if (result.failed) {
    m_debugger->sendUserMessage(
      result.error.c_str(),
      DebugTransport::OutputLevelError
    );
    throw DebuggerCommandException("Failed to evaluate expression.");
  }

  folly::dynamic serializedResult =
    VariablesCommand::serializeVariable(
      session,
      threadId,
      "",
      result.result
    );

  (*responseMsg)["body"] = folly::dynamic::object;
  folly::dynamic& body = (*responseMsg)["body"];
  body["result"] = serializedResult["value"];
  body["type"] = serializedResult["type"];

  int variableReference = tryGetInt(serializedResult, "variablesReference", -1);
  if (variableReference > 0) {
    body["variablesReference"] = serializedResult["variablesReference"];
  }

  int namedVariables = tryGetInt(serializedResult, "namedVariables", -1);
  if (namedVariables > 0) {
    body["namedVariables"] = serializedResult["namedVariables"];
  }

  int indexedVariables = tryGetInt(serializedResult, "indexedVariables", -1);
  if (indexedVariables > 0) {
    body["indexedVariables"] = serializedResult["indexedVariables"];
  }

  try {
    const auto& presentationHint = serializedResult["presentationHint"];
    body["presentationHint"] = presentationHint;
  } catch (std::out_of_range e) {
  }

  return false;
}

void EvaluateCommand::preparseEvalExpression(
  std::string* expr
) {
  // First, trim any leading and trailing white space.
  std::string& expression = *expr;
  expression = trimString(expression);

  // HPHPD users are used to having to prefix variable requests with a leading
  // = character. We don't require that, but tolorate that syntax to maintain
  // compatibility for those users.
  if (expression[0] == '=') {
    expression = expression.substr(1);
  }

  // If the user supplied an expression that looks like a well formed script,
  // meaning it begins with <?php or <?hh, do not do any further transformations
  // on it - we'll try to just evaluate it directly as the user intended, and
  // this will honor running as PHP vs Hack. Otherwise we are going to try
  // to interpret as Hack, and we need to turn this into a valid script snippet.
  std::string interpretExpr;
  bool runWithoutModifying;
  if (expression.find("<?php", 0, 5) == 0 ||
      expression.find("<?hh", 0, 4) == 0) {

    runWithoutModifying = true;
    interpretExpr = expression;
  } else {
    // In case the user entered just a bare variable name ("$x") to see a value,
    // append a ; to make the statement syntactically well formed.
    // NOTE: It is safe to do this even if the expression ends in a ; because
    //  $x;;
    // is still well-formed PHP. The parser removes the empty second statement.
    interpretExpr = "<?hh " + expression + ";";
    runWithoutModifying = false;
  }

  if (interpretExpr.empty()) {
    throw DebuggerCommandException("No expression provided to evaluate.");
  }

  String input(interpretExpr);
  AnalysisResultPtr ar(new AnalysisResult());
  StatementListPtr statements = Compiler::Parser::ParseString(input, ar);
  if (statements == nullptr) {
    throw DebuggerCommandException(
      "HHVM failed to parse the specified expression."
    );
  }

  if (statements->getCount() > 1) {
    if (!runWithoutModifying) {
      expression = interpretExpr;
    }
    return;
  }

  if (statements->getCount() == 0) {
    throw DebuggerCommandException("No expression provided to evaluate.");
  }

  // In the case of a single statement, if it is an expression, we need to
  // prepend "return" to it so that we get back the expression value the
  // user is likely expecting. Otherwise, we'll evaluate the expression,
  // and any side effects but end up returning void.
  StatementPtr statement = (*statements)[0];

  // Statement list says we have a single statement, expect one.
  assert(statement != nullptr);

  if (statement->getKindOf() == Construct::KindOfExpStatement) {
    interpretExpr = "<?hh ";
    interpretExpr += "return ";
    interpretExpr += expression;
    interpretExpr += ";";
  }

  if (!runWithoutModifying) {
    expression = interpretExpr;
  }
}

}
}
