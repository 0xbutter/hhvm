/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

// Get SIZE_MAX definition.  Do this before including any other files, to make
// sure that this is the first place that stdint.h is included.
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include <runtime/base/memory/memory_manager.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/memory/leak_detectable.h>
#include <runtime/base/memory/sweepable.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/http_server.h>
#include <util/alloc.h>
#include <util/process.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#ifdef USE_JEMALLOC
bool MemoryManager::s_statsEnabled = false;
size_t MemoryManager::s_cactiveLimitCeiling = 0;

static size_t threadAllocatedpMib[2];
static size_t threadDeallocatedpMib[2];
static size_t statsCactiveMib[2];
static pthread_once_t threadStatsOnce = PTHREAD_ONCE_INIT;
static void threadStatsInit() {
  if (!mallctlnametomib) return;
  size_t miblen = sizeof(threadAllocatedpMib) / sizeof(size_t);
  if (mallctlnametomib("thread.allocatedp", threadAllocatedpMib, &miblen)) {
    return;
  }
  miblen = sizeof(threadDeallocatedpMib) / sizeof(size_t);
  if (mallctlnametomib("thread.deallocatedp", threadDeallocatedpMib, &miblen)) {
    return;
  }
  miblen = sizeof(statsCactiveMib) / sizeof(size_t);
  if (mallctlnametomib("stats.cactive", statsCactiveMib, &miblen)) {
    return;
  }
  MemoryManager::s_statsEnabled = true;

  // In threadStats() we wish to solve for cactiveLimit in:
  //
  //   footprint + cactiveLimit + headRoom == MemTotal
  //
  // However, headRoom comes from RuntimeOption::ServerMemoryHeadRoom, which
  // isn't initialized until after the code here runs.  Therefore, compute
  // s_cactiveLimitCeiling here in order to amortize the cost of introspecting
  // footprint and MemTotal.
  //
  //   cactiveLimit == (MemTotal - footprint) - headRoom
  //
  //   cactiveLimit == s_cactiveLimitCeiling - headRoom
  // where
  //   s_cactiveLimitCeiling == MemTotal - footprint
  size_t footprint = Process::GetCodeFootprint(Process::GetProcessId());
  size_t pageSize = size_t(sysconf(_SC_PAGESIZE));
  size_t MemTotal = size_t(sysconf(_SC_PHYS_PAGES)) * pageSize;
  if (MemTotal > footprint) {
    MemoryManager::s_cactiveLimitCeiling = MemTotal - footprint;
  }
}

static inline void threadStats(uint64*& allocated, uint64*& deallocated,
                               size_t*& cactive, size_t& cactiveLimit) {
  pthread_once(&threadStatsOnce, threadStatsInit);
  if (!MemoryManager::s_statsEnabled) return;

  size_t len = sizeof(allocated);
  if (mallctlbymib(threadAllocatedpMib,
                   sizeof(threadAllocatedpMib) / sizeof(size_t),
                   &allocated, &len, NULL, 0)) {
    not_reached();
  }

  len = sizeof(deallocated);
  if (mallctlbymib(threadDeallocatedpMib,
                   sizeof(threadDeallocatedpMib) / sizeof(size_t),
                   &deallocated, &len, NULL, 0)) {
    not_reached();
  }

  len = sizeof(cactive);
  if (mallctlbymib(statsCactiveMib,
                   sizeof(statsCactiveMib) / sizeof(size_t),
                   &cactive, &len, NULL, 0)) {
    not_reached();
  }

  size_t headRoom = RuntimeOption::ServerMemoryHeadRoom;
  // Compute cactiveLimit based on s_cactiveLimitCeiling, as computed in
  // threadStatsInit().
  if (headRoom != 0 && headRoom < MemoryManager::s_cactiveLimitCeiling) {
    cactiveLimit = MemoryManager::s_cactiveLimitCeiling - headRoom;
  } else {
    cactiveLimit = SIZE_MAX;
  }
}
#endif

static void* MemoryManagerInit() {
  // We store the free list pointers right at the start of each object,
  // overlapping SmartHeader.data, and we also clobber _count as a
  // free-object flag when the object is deallocated (if hhvm).
  // This assert just makes sure they don't overflow.
  static_assert(FAST_REFCOUNT_OFFSET + sizeof(int) <=
                SmartAllocatorImpl::MinItemSize,
                "MinItemSize is too small");
  MemoryManager::TlsWrapper tls;
  return (void*)tls.getNoCheck;
}

void* MemoryManager::TlsInitSetup = MemoryManagerInit();

void MemoryManager::Create(void* storage) {
  new (storage) MemoryManager();
}

void MemoryManager::Delete(MemoryManager* mm) {
  mm->~MemoryManager();
}

void MemoryManager::OnThreadExit(MemoryManager* mm) {
  mm->~MemoryManager();
}

MemoryManager::AllocIterator::AllocIterator(const MemoryManager* mman)
  : m_mman(*mman)
  , m_it(m_mman.m_smartAllocators.begin())
{}

SmartAllocatorImpl*
MemoryManager::AllocIterator::current() const {
  return m_it == m_mman.m_smartAllocators.end() ? 0 : *m_it;
}

void MemoryManager::AllocIterator::next() {
  ++m_it;
}

MemoryManager::MemoryManager() : m_enabled(RuntimeOption::EnableMemoryManager) {
#ifdef USE_JEMALLOC
  threadStats(m_allocated, m_deallocated, m_cactive, m_cactiveLimit);
#endif
  resetStats();
  m_stats.maxBytes = INT64_MAX;
  m_front = m_limit = 0;
  m_smartsweep = 0;
}

void MemoryManager::resetStats() {
  m_stats.usage = 0;
  m_stats.alloc = 0;
  m_stats.peakUsage = 0;
  m_stats.peakAlloc = 0;
  m_stats.totalAlloc = 0;
#ifdef USE_JEMALLOC
  if (s_statsEnabled) {
#ifdef HHVM
    m_stats.jemallocDebt = 0;
#endif
    m_prevAllocated = int64(*m_allocated);
    m_delta = m_prevAllocated - int64(*m_deallocated);
  }
#endif
}

NEVER_INLINE
void MemoryManager::refreshStatsHelper() {
  refreshStats();
}

void MemoryManager::refreshStatsHelperExceeded() {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  info->m_reqInjectionData.setMemExceededFlag();
}

#ifdef USE_JEMALLOC
void MemoryManager::refreshStatsHelperStop() {
  HttpServer::Server->stop();
  // Increase the limit to the maximum possible value, so that this method
  // won't be called again.
  m_cactiveLimit = SIZE_MAX;
}
#endif

void MemoryManager::add(SmartAllocatorImpl *allocator) {
  ASSERT(allocator);
  m_smartAllocators.push_back(allocator);
}

void MemoryManager::sweepAll() {
  Sweepable::SweepAll();
#ifdef HHVM_GC
  GCRootTracker<StringData>::clear();
  GCRootTracker<ArrayData>::clear();
  GCRootTracker<ObjectData>::clear();
  GCRootTracker<Variant>::clear();
  GCRoot<StringData>::clear();
  GCRoot<ArrayData>::clear();
  GCRoot<ObjectData>::clear();
  GCRoot<Variant>::clear();
#endif
}

struct SmallNode {
  size_t padbytes; // <= kMaxSmartSize means small block
};

struct SweepNode {
  SweepNode* next;
  union {
    SweepNode* prev;
    size_t padbytes;
  };
};

void MemoryManager::rollback() {
  typedef std::vector<char*>::const_iterator SlabIter;
  for (unsigned int i = 0; i < m_smartAllocators.size(); i++) {
    m_smartAllocators[i]->rollbackObjects();
  }
  // free smart-malloc slabs
  for (SlabIter i = m_slabs.begin(), end = m_slabs.end(); i != end; ++i) {
    free(*i);
  }
  m_slabs.clear();
  // free large allocation blocks
  if (SweepNode* n = m_smartsweep) {
    for (SweepNode *next = 0; next != m_smartsweep; n = next) {
      next = n->next;
      free(n);
    }
    m_smartsweep = 0;
  }
  // zero out freelists
  for (unsigned i = 0; i < kNumSizes; i++) {
    m_smartfree[i].clear();
  }
  m_front = m_limit = 0;
}

void MemoryManager::logStats() {
  LeakDetectable::LogMallocStats();
}

void MemoryManager::checkMemory(bool detailed) {
  printf("----- MemoryManager for Thread %ld -----\n", (long)pthread_self());

  refreshStats();
  printf("Current Usage: %lld bytes\t", m_stats.usage);
  printf("Current Alloc: %lld bytes\n", m_stats.alloc);
  printf("Peak Usage: %lld bytes\t", m_stats.peakUsage);
  printf("Peak Alloc: %lld bytes\n", m_stats.peakAlloc);

  printf("Slabs: %lu KiB\n", m_slabs.size() * SLAB_SIZE / 1024);

  for (unsigned int i = 0; i < m_smartAllocators.size(); i++) {
    m_smartAllocators[i]->checkMemory(detailed);
  }
}

//
// smart_malloc implementation notes
//
// These functions allocate all small blocks from a single slab,
// and defer larger allocations directly to malloc.  When small blocks
// are freed they're placed the appropriate size-segreated freelist.
// (m_smartfree[i]).  Small blocks have an 8-byte SmallNode and
// are swept en-masse when slabs are freed.
//
// Medium blocks use a 16-byte SweepNode header to maintain a doubly-linked
// list of blocks to free at request end.  smart_free can distinguish
// SmallNode and SweepNode because valid next/prev pointers must be
// larger than kMaxSmartSize.
//

inline void* MemoryManager::smartMalloc(size_t nbytes) {
  ASSERT(nbytes > 0);
  size_t padbytes = (nbytes + sizeof(SmallNode) + kMask) & ~kMask;
  if (LIKELY(padbytes <= kMaxSmartSize)) {
    // add room for header before rounding up, so the header always is
    // 8-byte aligned and the usable memory is always 16-aligned.
    m_stats.usage += padbytes;
    unsigned i = (padbytes - 1) >> kLgSizeQuantum;
    ASSERT(i < kNumSizes);
    void* p = m_smartfree[i].maybePop();
    if (LIKELY(p != 0)) return p;
    char* mem = m_front;
    if (LIKELY(mem + padbytes <= m_limit)) {
      m_front = mem + padbytes;
      SmallNode* n = (SmallNode*) mem;
      n->padbytes = padbytes;
      return n + 1;
    }
    return smartMallocSlab(padbytes);
  }
  return smartMallocBig(nbytes);
}

inline void MemoryManager::smartFree(void* ptr) {
  ASSERT(ptr != 0);
  SweepNode* n = ((SweepNode*)ptr) - 1;
  size_t padbytes = n->padbytes;
  if (LIKELY(padbytes <= kMaxSmartSize)) {
    ASSERT(memset(ptr, kSmartFreeFill, padbytes - sizeof(SmallNode)));
    unsigned i = (padbytes - 1) >> kLgSizeQuantum;
    ASSERT(i < kNumSizes);
    m_smartfree[i].push(ptr);
    m_stats.usage -= padbytes + sizeof(SmallNode);
    return;
  }
  smartFreeBig(n);
}

// quick-and-dirty realloc implementation.  We could do better if the block
// is malloc'd, by deferring to the underlying realloc.
inline void* MemoryManager::smartRealloc(void* ptr, size_t nbytes) {
  ASSERT(ptr != 0 && nbytes > 0);
  SweepNode* n = ((SweepNode*)ptr) - 1;
  size_t old_padbytes = n->padbytes;
  if (LIKELY(old_padbytes <= kMaxSmartSize)) {
    void* newmem = smartMalloc(nbytes);
    memcpy(newmem, ptr, std::min(old_padbytes, nbytes));
    smartFree(ptr);
    return newmem;
  }
  SweepNode* next = n->next;
  SweepNode* prev = n->prev;
  SweepNode* n2 = (SweepNode*) realloc(n, nbytes + sizeof(SweepNode));
  if (n2 != n) {
    // block moved; must re-link to sweeplist
    if (next != n) {
      next->prev = prev->next = n2;
    } else {
      n2->next = n2->prev = n2;
    }
    if (m_smartsweep == n) m_smartsweep = n2;
  }
  return n2 + 1;
}

NEVER_INLINE char* MemoryManager::newSlab() {
  if (hhvm && UNLIKELY(m_stats.usage > m_stats.maxBytes)) {
    refreshStatsHelper();
  }
  char* slab = (char*) Util::safe_malloc(SLAB_SIZE);
  JEMALLOC_STATS_ADJUST(&m_stats, SLAB_SIZE);
  m_stats.alloc += SLAB_SIZE;
  if (m_stats.alloc > m_stats.peakAlloc) {
    m_stats.peakAlloc = m_stats.alloc;
  }
  m_slabs.push_back(slab);
  return slab;
}

NEVER_INLINE
void* MemoryManager::smartMallocSlab(size_t padbytes) {
  char* slab = newSlab();
  // padding 8 bytes here aligns the usable area of smart_malloc blocks
  // on 16-byte boundaries.
  size_t align = sizeof(SmallNode) & 15;
  m_front = slab + align + padbytes;
  m_limit = slab + SLAB_SIZE;
  SmallNode* n = (SmallNode*) (slab + align);
  n->padbytes = padbytes;
  return n + 1;
}

inline void* MemoryManager::smartEnlist(SweepNode* n) {
  if (hhvm && UNLIKELY(m_stats.usage > m_stats.maxBytes)) {
    refreshStatsHelper();
  }
  SweepNode* next = m_smartsweep;
  if (next) {
    SweepNode* prev = next->prev;
    n->next = next;
    n->prev = prev;
    next->prev = prev->next = n;
  } else {
    n->next = n->prev = n;
  }
  m_smartsweep = n;
  ASSERT(n->padbytes > kMaxSmartSize);
  return n + 1;
}

NEVER_INLINE
void* MemoryManager::smartMallocBig(size_t nbytes) {
  ASSERT(nbytes > 0);
  SweepNode* n = (SweepNode*) Util::safe_malloc(nbytes + sizeof(SweepNode));
  return smartEnlist(n);
}

NEVER_INLINE
void* MemoryManager::smartCallocBig(size_t totalbytes) {
  ASSERT(totalbytes > 0);
  SweepNode* n = (SweepNode*)Util::safe_calloc(totalbytes + sizeof(SweepNode),
                                               1);
  return smartEnlist(n);
}

NEVER_INLINE
void MemoryManager::smartFreeBig(SweepNode* n) {
  SweepNode* next = n->next;
  SweepNode* prev = n->prev;
  next->prev = prev;
  prev->next = next;
  if (UNLIKELY(n == m_smartsweep)) {
    m_smartsweep = (next != n) ? next : 0;
  }
  free(n);
}

static inline MemoryManager& MM() {
  return *MemoryManager::TheMemoryManager();
}

// smart_malloc api entry points, with support for malloc/free corner cases.

HOT_FUNC
void* smart_malloc(size_t nbytes) {
  return MM().smartMalloc(std::max(nbytes, size_t(1)));
}

HOT_FUNC
void* smart_calloc(size_t count, size_t nbytes) {
  size_t totalbytes = std::max(nbytes * count, size_t(1));
  if (totalbytes <= MemoryManager::kMaxSmartSize) {
    return memset(MM().smartMalloc(totalbytes), 0, totalbytes);
  }
  return MM().smartCallocBig(totalbytes);
}

HOT_FUNC
void* smart_realloc(void* ptr, size_t nbytes) {
  if (!ptr) return MM().smartMalloc(std::max(nbytes, size_t(1)));
  if (!nbytes) return ptr ? MM().smartFree(ptr), (void*)0 : (void*)0;
  return MM().smartRealloc(ptr, nbytes);
}

HOT_FUNC
void smart_free(void* ptr) {
  if (ptr) MM().smartFree(ptr);
}

///////////////////////////////////////////////////////////////////////////////
}
