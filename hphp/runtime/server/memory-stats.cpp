/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/server/memory-stats.h"

#include <stdio.h>
#include <string>
#include <sstream>

#include "hphp/runtime/server/writer.h"



namespace HPHP{

  void MemoryStats::ReportMemory(std::string &output, Writer::Format format){
    std::ostringstream out;
    Writer *w;
    if (format == Writer::Format::XML) {
      w = new XMLWriter(out);
    } else if (format == Writer::Format::HTML) {
      w = new HTMLWriter(out);
    } else {
      assert(format == Writer::Format::JSON);
      w = new JSONWriter(out);
    }

    w->writeFileHeader();
    w->beginObject("Memory");
    w->endObject("Memory");
    w->writeFileFooter();
    delete w;

    output = out.str();
    return;
  }
}
