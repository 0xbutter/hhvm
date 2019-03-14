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

#include "hphp/compiler/analysis/analysis_result.h"

#include <folly/Conv.h>
#include <folly/portability/SysStat.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "hphp/compiler/analysis/exceptions.h"
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/package.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/util/hash.h"
#include "hphp/util/job-queue.h"
#include "hphp/util/logger.h"
#include "hphp/util/timer.h"

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////
// initialization

AnalysisResult::AnalysisResult()
  : m_package(nullptr), m_parseOnDemand(false), m_phase(ParseAllFiles),
    m_asmCallbacks(AnalysisResultConstRawPtr{this}) {
}

AnalysisResult::~AnalysisResult() {
  always_assert(!m_finish);
}

void AnalysisResult::finish() {
  if (m_finish) {
    decltype(m_finish) f;
    f.swap(m_finish);
    f(shared_from_this());
  }
}

///////////////////////////////////////////////////////////////////////////////
// general functions

void AnalysisResult::addHhasFile(std::unique_ptr<UnitEmitter>&& ue) {
  const uint64_t sz = m_hhasFiles.size();
  ue->setMd5(MD5 { sz });
  m_hhasFiles.emplace_back(std::move(ue));
}

std::vector<std::unique_ptr<UnitEmitter>> AnalysisResult::getHhasFiles() {
  return std::move(m_hhasFiles);
}

bool AnalysisResult::inParseOnDemandDirs(const std::string &filename) const {
  for (size_t i = 0; i < m_parseOnDemandDirs.size(); i++) {
    if (filename.find(m_parseOnDemandDirs[i]) == 0) return true;
  }
  return false;
}

void AnalysisResult::parseOnDemand(const std::string &name) const {
  if (m_package) {
    auto const& root = m_package->getRoot();
    auto rname = name;
    if (name.find(root) == 0) {
      rname = name.substr(root.length());
    }
    if ((m_parseOnDemand || inParseOnDemandDirs(rname)) &&
        Option::PackageExcludeFiles.find(rname) ==
        Option::PackageExcludeFiles.end() &&
        !Option::IsFileExcluded(rname, Option::PackageExcludePatterns)) {
      m_package->addSourceFile(rname);
    }
  }
}

template <class Map>
void AnalysisResult::parseOnDemandBy(const std::string &name,
                                     const Map &amap) const {
  if (m_package) {
    auto it = amap.find(name);
    if (it != amap.end()) {
      parseOnDemand(Option::AutoloadRoot + it->second);
    }
  }
}

template void AnalysisResult::parseOnDemandBy(
  const std::string &name, const std::map<std::string,std::string> &amap) const;

template void AnalysisResult::parseOnDemandBy(
  const std::string &name,
  const std::map<std::string,std::string,stdltistr> &amap) const;

///////////////////////////////////////////////////////////////////////////////
// Program

void AnalysisResult::dump() {
  fflush(0);
}

///////////////////////////////////////////////////////////////////////////////
// code generation functions

std::string AnalysisResult::prepareFile(const char *root,
                                        const std::string &fileName,
                                        bool chop,
                                        bool stripPath /* = true */) {
  std::string fullPath = root;
  if (!fullPath.empty() &&
    !FileUtil::isDirSeparator(fullPath[fullPath.size() - 1])) {
    fullPath += FileUtil::getDirSeparator();
  }

  auto file = fileName;
  if (stripPath) {
    size_t npos = file.rfind('/');
    if (npos != std::string::npos) {
      file = file.substr(npos + 1);
    }
  }

  if (chop && file.size() > 4 && file.substr(file.length() - 4) == ".php") {
    fullPath += file.substr(0, file.length() - 4);
  } else {
    fullPath += file;
  }
  for (size_t pos = strlen(root); pos < fullPath.size(); pos++) {
    if (FileUtil::isDirSeparator(fullPath[pos])) {
      mkdir(fullPath.substr(0, pos).c_str(), 0777);
    }
  }
  return fullPath;
}
