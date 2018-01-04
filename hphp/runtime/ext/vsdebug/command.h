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

#ifndef incl_HPHP_VSDEBUG_COMMAND_H_
#define incl_HPHP_VSDEBUG_COMMAND_H_

#include <folly/dynamic.h>
#include <folly/json.h>

namespace HPHP {
namespace VSDEBUG {

// Forward declaration of Debugger
struct Debugger;

// Enum describing the target to which a debugger command needs to
// be dispatched for exection.
enum CommandTarget {
  // A command targeting "Request" needs to be handed off to a particular
  // request thread that is the target of the command.
  Request,

  // A command targeting "Dummy" needs to be processed in the context of the
  // dummy request thread.
  Dummy,

  // A command targeting "None" should be executed inline as soon as
  // it is received.
  None,
};

// This serves as the base class for all VS Code Debug Protocol commands that
// can be issued from the debugger client.
struct VSCommand {
  virtual ~VSCommand();

  // Returns the name of the command.
  virtual const char* commandName() = 0;

  // Returns the target type of the command.
  virtual CommandTarget commandTarget() = 0;

  // Returns true if this command can only be executed when the program is
  // broken in to the debugger.
  virtual bool requiresBreak() = 0;

  // Returns the ID of the request thread this command is directed at.
  virtual int64_t targetThreadId() = 0;

  // Executes the command. Returns true if the target thread should resume.
  bool execute();

  folly::dynamic& getMessage() { return m_message; }

  // Takes in a JSON message from the attached debugger client, parses it and
  // returns an executable debugger command.
  // Returns true if the message was successfully parsed into a command, false
  // if the message was invalid (or the command is not supported).
  static bool parseCommand(
    Debugger* debugger,
    folly::dynamic& clientMessage,
    VSCommand** command);

protected:

  // Implemented by subclasses of this object.
  virtual bool executeImpl(folly::dynamic* responseMsg) = 0;

  // Copy of the original JSON object sent by the client.
  folly::dynamic m_message;

private:
  VSCommand(Debugger* debugger, folly::dynamic message);

  Debugger* m_debugger;
};

}
}

#endif // incl_HPHP_VSDEBUG_COMMAND_H_
