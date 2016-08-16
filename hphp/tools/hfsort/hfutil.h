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

#include <limits>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "hphp/util/hash.h"

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

using TargetId = int32_t;
constexpr int32_t InvalidId = -1;
constexpr uint64_t InvalidAddr = std::numeric_limits<uint64_t>::max();

struct Arc {
  Arc(TargetId s, TargetId d, double w = 0)
      : src(s)
      , dst(d)
      , weight(w)
  {}
  Arc(const Arc&) = delete;

  friend bool operator==(const Arc& lhs, const Arc& rhs) {
    return lhs.src == rhs.src && lhs.dst == rhs.dst;
  }

  const TargetId src;
  const TargetId dst;
  mutable double weight;
  mutable double normalizedWeight{0};
  mutable double avgCallOffset{0};
};

struct ArcHash {
  int64_t operator()(const Arc& arc) const {
    return hash_int64_pair(int64_t(arc.src), int64_t(arc.dst));
  }
};

struct Target {
  explicit Target(uint32_t size, uint32_t samples = 0)
    : size(size)
    , samples(samples)
  {}

  uint32_t size;
  uint32_t samples;

  // preds and succs contain no duplicate elements and self arcs are not allowed
  std::vector<TargetId> preds;
  std::vector<TargetId> succs;
};

struct TargetGraph {
  TargetId addTarget(uint32_t size, uint32_t samples = 0);
  const Arc& incArcWeight(TargetId src, TargetId dst, double w = 1.0);

  std::vector<Target> targets;
  std::unordered_set<Arc, ArcHash> arcs;
};

struct Cluster {
  Cluster(TargetId id, const Target& f);

  std::string toString() const;

  std::vector<TargetId> targets;
  uint32_t samples;
  double arcWeight; // intra-cluster callgraph arc weight
  uint32_t size;
  bool frozen; // not a candidate for merging
};

struct Func {
  Func(std::string name, uint64_t a, uint32_t g)
    : group(g)
    , addr(a)
    , mangledNames(1, name)
  {}

  bool valid() const { return mangledNames.size() > 0; }
  const uint32_t group;
  const uint64_t addr;
  std::vector<std::string> mangledNames;
};

struct CallGraph : TargetGraph {
  bool addFunc(std::string name, uint64_t addr, uint32_t size, uint32_t group);
  TargetId addrToTargetId(uint64_t addr) const;
  void printDot(char* fileName) const;

  std::string toString(TargetId id) const;

  std::vector<Func> funcs;
  std::map<uint64_t,TargetId> addr2TargetId;
};

bool compareClustersDensity(const Cluster& c1, const Cluster& c2);
std::vector<Cluster> clusterize(const TargetGraph& cg);

/*
 * Pettis-Hansen code layout algorithm
 * reference: K. Pettis and R. C. Hansen, "Profile Guided Code Positioning",
 *            PLDI '90
 */
std::vector<Cluster> pettisAndHansen(const TargetGraph& cg);

}}

#endif
