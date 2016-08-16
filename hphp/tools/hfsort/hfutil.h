/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_FACEBOOK_HFSORT_HFUTIL_H
#define incl_HPHP_FACEBOOK_HFSORT_HFUTIL_H

#include <folly/Format.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

namespace HPHP { namespace hfsort {

// The number of pages to reserve for the functions with highest
// density (samples / size).  The functions put in these pages are not
// considered for clustering.
constexpr uint32_t kFrozenPages = 0;

// The minimum approximate probability of a callee being called from a
// particular arc to consider merging with the caller's cluster.
constexpr double kMinArcProbability = 0.1;

// This is a factor to determine by how much a caller cluster is
// willing to degrade it's density by merging a callee.
constexpr int kCallerDegradeFactor = 8;

// Maximum size of a cluster, in bytes.
constexpr uint32_t kMaxClusterSize = 1 << 20;

constexpr uint32_t kPageSize = 2 << 20;

constexpr uint32_t BUFLEN = 1000;

// Change this and recompile to change tracing level.
constexpr uint8_t tracing = 1;

// Supported code layout algorithms
enum class Algorithm { Hfsort, PettisHansen, Invalid };

void trace(const char* fmt, ...);
#define HFTRACE(LEVEL, ...)                  \
  if (tracing >= LEVEL) { trace(__VA_ARGS__); }

using FuncId = int32_t;
constexpr int32_t InvalidId = -1;
constexpr uint64_t InvalidAddr = std::numeric_limits<uint64_t>::max();

struct Arc {
  Arc(FuncId s, FuncId d, double w)
      : src(s)
      , dst(d)
      , weight(w)
  {}

  FuncId src;
  FuncId dst;
  double weight;
  double normalizedWeight{0};
  double avgCallOffset{0};
};

struct Func {
  Func(FuncId id, std::string name, uint64_t a, uint32_t s, uint32_t g)
      : id(id)
      , group(g)
      , addr(a)
      , size(s) {
    mangledNames.push_back(name);
  }

  bool valid() const { return mangledNames.size() > 0; }
  std::string toString() const;

  FuncId id;
  uint32_t group;
  uint64_t addr;
  uint32_t size;
  uint32_t samples{0};
  std::vector<std::string> mangledNames;
  std::vector<Arc*> inArcs;
  std::vector<Arc*> outArcs;
};

struct CallGraph {
  ~CallGraph();

  bool addFunc(Func f);
  Arc* findArc(FuncId src, FuncId dst) const;
  Arc* getArc(FuncId src, FuncId dst);
  Arc* incArcWeight(FuncId src, FuncId dst, double w = 1.0);
  FuncId addrToFuncId(uint64_t addr) const;
  void printDot(char *fileName) const;

  std::vector<Func> funcs;
  std::map<uint64_t,FuncId> addr2FuncId;
  std::vector<Arc*> arcs;
};

struct Cluster {
  explicit Cluster(const Func& f);

  void merge(const Cluster& other, const double aw = 0);
  std::string toString() const;

  std::vector<FuncId> funcs;
  uint32_t samples;
  double arcWeight; // intra-cluster callgraph arc weight
  uint32_t size;
  bool frozen; // not a candidate for merging
};

bool compareClustersDensity(const Cluster* c1, const Cluster* c2);
std::vector<Cluster*> clusterize(const CallGraph& cg);

/*
 * Pettis-Hansen code layout algorithm
 * reference: K. Pettis and R. C. Hansen, "Profile Guided Code Positioning",
 *            PLDI '90
 */
std::vector<Cluster*> pettisAndHansen(const CallGraph& cg);

}}

#endif
