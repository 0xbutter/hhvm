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
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/memory-manager-defs.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/safe-cast.h"
#include "hphp/util/trace.h"

namespace HPHP {

TRACE_SET_MOD(mm);

void SparseHeap::threadInit() {
  m_slabManager = SlabManager::get(s_numaNode);
}

void SparseHeap::reset() {
  TRACE(1, "heap-id %lu SparseHeap-reset: slabs %lu bigs %lu\n",
        tl_heap_id, m_slabs.size(), m_bigs.size());
  if (RuntimeOption::EvalTrashFillOnRequestExit) {
    for (auto slab : m_slabs) {
      memset(slab.ptr, kSmallFreeFill, slab.size);
    }
    for (auto n : m_bigs) {
      memset(n + 1, kSmallFreeFill, n->nbytes - sizeof(*n));
    }
  }
  auto const do_free = [](void* ptr, size_t size) {
#if defined(USE_JEMALLOC) && (JEMALLOC_VERSION_MAJOR >= 4)
    sdallocx(ptr, size, 0);
#else
    free(ptr);
#endif
  };
  TaggedSlabList pooledSlabs;
  void* pooledSlabTail = nullptr;
  for (auto slab : m_slabs) {
    // The first slab contains the php stack, so only unmap it when the worker
    // thread dies.
    if (slab.ptr == s_firstSlab.ptr) continue;
    if (slab.pooled) {
      if (!pooledSlabTail) pooledSlabTail = slab.ptr;
      pooledSlabs.push_front<true>(slab.ptr, slab.version);
    } else {
      // The only slab with irregular size is the first slab.
      do_free(slab.ptr, kSlabSize);
    }
  }
  if (pooledSlabTail) {
    m_slabManager->merge(pooledSlabs.head(), pooledSlabTail);
  }
  m_slabs.clear();
  m_pooledBytes = 0;
  for (auto n : m_bigs) do_free(n, n->nbytes);
  m_bigs.clear();
}

void SparseHeap::flush() {
  assertx(empty());
  m_slabs = std::vector<SlabInfo>{};
  m_bigs = std::vector<MallocNode*>{};
  m_pooledBytes = 0;
}

HeapObject* SparseHeap::allocSlab(MemoryUsageStats& stats) {
  // If we have a pre-allocated slab, and it's slab-aligned, use it first.
  if (m_slabs.empty() && s_firstSlab.size >= kMaxSmallSize &&
      reinterpret_cast<uintptr_t>(s_firstSlab.ptr) % kSlabAlign == 0) {
    auto const slabSize = s_firstSlab.size;
    stats.mmap_volume += slabSize;
    stats.mmap_cap += slabSize;
    stats.peakCap = std::max(stats.peakCap, stats.capacity());
    m_slabs.emplace_back(s_firstSlab.ptr, slabSize);
    return (HeapObject*)s_firstSlab.ptr;
  }
  if (m_slabManager && m_pooledBytes < RuntimeOption::RequestHugeMaxBytes) {
    if (auto slab = m_slabManager->tryAlloc()) {
      stats.mmap_volume += kSlabSize;
      stats.mmap_cap += kSlabSize;
      stats.peakCap = std::max(stats.peakCap, stats.capacity());
      m_slabs.emplace_back(slab.ptr(), kSlabSize, slab.tag());
      m_pooledBytes += kSlabSize;
      return (HeapObject*)slab.ptr();
    }
  }
#ifdef USE_JEMALLOC
  void* slab = mallocx(kSlabSize, MALLOCX_ALIGN(kSlabAlign));
  auto usable = sallocx(slab, 0);
#else
  auto slab = safe_aligned_alloc(kSlabAlign, kSlabSize);
  auto usable = kSlabSize;
#endif
  m_slabs.emplace_back(slab, kSlabSize);
  stats.malloc_cap += usable;
  stats.peakCap = std::max(stats.peakCap, stats.capacity());
  return (HeapObject*)slab;
}

void SparseHeap::enlist(MallocNode* n, HeaderKind kind,
                        size_t size, type_scan::Index tyindex) {
  n->initHeader_32_16(kind, m_bigs.size(), tyindex);
  n->nbytes = size;
  m_bigs.push_back(n);
}

void* SparseHeap::allocBig(size_t bytes, HeaderKind kind,
                           type_scan::Index tyindex,
                           MemoryUsageStats& stats) {
#ifdef USE_JEMALLOC
  auto n = static_cast<MallocNode*>(mallocx(bytes + sizeof(MallocNode), 0));
  auto cap = sallocx(n, 0);
#else
  auto n = static_cast<MallocNode*>(safe_malloc(bytes + sizeof(MallocNode)));
  auto cap = malloc_usable_size(n);
#endif
  enlist(n, kind, bytes + sizeof(MallocNode), tyindex);
  stats.mm_udebt -= cap;
  stats.malloc_cap += cap;
  return n + 1;
}

void* SparseHeap::callocBig(size_t nbytes, HeaderKind kind,
                            type_scan::Index tyindex,
                            MemoryUsageStats& stats) {
#ifdef USE_JEMALLOC
  auto n = static_cast<MallocNode*>(
      mallocx(nbytes + sizeof(MallocNode), MALLOCX_ZERO)
  );
  auto cap = sallocx(n, 0);
#else
  auto n = static_cast<MallocNode*>(
      safe_calloc(nbytes + sizeof(MallocNode), 1)
  );
  auto cap = malloc_usable_size(n);
#endif
  enlist(n, kind, nbytes + sizeof(MallocNode), tyindex);
  stats.mm_udebt -= cap;
  stats.malloc_cap += cap;
  return n + 1;
}

bool SparseHeap::contains(void* ptr) const {
  auto const ptrInt = reinterpret_cast<uintptr_t>(ptr);
  auto it = std::find_if(std::begin(m_slabs), std::end(m_slabs),
    [&] (const SlabInfo& slab) {
      auto const baseInt = reinterpret_cast<uintptr_t>(slab.ptr);
      return ptrInt >= baseInt && ptrInt < baseInt + slab.size;
    }
  );
  return it != std::end(m_slabs);
}

void SparseHeap::freeBig(void* ptr, MemoryUsageStats& stats) {
  auto n = static_cast<MallocNode*>(ptr) - 1;
  auto i = n->index();
  auto last = m_bigs.back();
  last->index() = i;
  m_bigs[i] = last;
  m_bigs.pop_back();
#ifdef USE_JEMALLOC
  auto cap = sallocx(n, 0);
  dallocx(n, 0);
#else
  auto cap = malloc_usable_size(n);
  free(n);
#endif
  // Since we account for these direct allocations in our usage and adjust for
  // them on allocation, we also need to adjust for them negatively on free.
  stats.mm_freed += cap;
  stats.malloc_cap -= cap;
}

void* SparseHeap::resizeBig(void* ptr, size_t nbytes,
                            MemoryUsageStats& stats) {
  // Since we don't know how big it is (i.e. how much data we should memcpy),
  // we have no choice but to ask malloc to realloc for us.
  auto const n = static_cast<MallocNode*>(ptr) - 1;
  auto new_size = nbytes + sizeof(MallocNode);
#ifdef USE_JEMALLOC
  auto old_cap = sallocx(n, 0);
  auto const newNode = static_cast<MallocNode*>(
    rallocx(n, new_size, 0)
  );
  auto new_cap = sallocx(newNode, 0);
#else
  auto old_cap = malloc_usable_size(n);
  auto const newNode = static_cast<MallocNode*>(
    safe_realloc(n, new_size)
  );
  auto new_cap = malloc_usable_size(newNode);
#endif
  newNode->nbytes = new_size;
  if (newNode != n) {
    m_bigs[newNode->index()] = newNode;
  }
  stats.mm_udebt -= new_cap - old_cap;
  stats.malloc_cap += new_cap - old_cap;
  return newNode + 1;
}

void SparseHeap::sort() {
  std::sort(std::begin(m_slabs), std::end(m_slabs),
    [] (const SlabInfo& l, const SlabInfo& r) {
      assertx(static_cast<char*>(l.ptr) + l.size <= r.ptr ||
              static_cast<char*>(r.ptr) + r.size <= l.ptr);
      return l.ptr < r.ptr;
    }
  );
  std::sort(std::begin(m_bigs), std::end(m_bigs));
  for (size_t i = 0, n = m_bigs.size(); i < n; ++i) {
    m_bigs[i]->index() = i;
  }
}

/*
 * To find `p', we sort the slabs, bisect them, then iterate the slab
 * containing `p'.  If there is no such slab, we bisect the bigs to try to find
 * a big containing `p'.
 *
 * If that fails, we return nullptr.
 */
HeapObject* SparseHeap::find(const void* p) {
  sort();
  auto const slab = std::lower_bound(
    std::begin(m_slabs), std::end(m_slabs), p,
    [] (const SlabInfo& slab, const void* p) {
      return static_cast<const char*>(slab.ptr) + slab.size <= p;
    }
  );

  if (slab != std::end(m_slabs) && slab->ptr <= p) {
    // std::lower_bound() finds the first slab that is not less than `p'.  By
    // our comparison predicate, a slab is less than `p' iff its entire range
    // is below `p', so if the returned slab's start address is <= `p', then
    // the slab must contain `p'.  Within the slab, we just do a linear search.
    auto h = reinterpret_cast<char*>(slab->ptr);
    auto const slab_end = h + slab->size;
    while (h < slab_end) {
      auto const hdr = reinterpret_cast<HeapObject*>(h);
      auto const size = allocSize(hdr);
      if (p < h + size) return hdr;
      h += size;
    }
    // We know `p' is in the slab, so it must belong to one of the headers.
    always_assert(false);
  }

  auto const big = std::lower_bound(
    std::begin(m_bigs), std::end(m_bigs), p,
    [] (const MallocNode* big, const void* p) {
      return reinterpret_cast<const char*>(big) + big->nbytes <= p;
    }
  );

  if (big != std::end(m_bigs) && *big <= p) {
    auto const hdr = reinterpret_cast<HeapObject*>(*big);
    if (hdr->kind() != HeaderKind::BigObj) {
      // `p' is part of the MallocNode.
      return hdr;
    }
    auto const sub = reinterpret_cast<HeapObject*>(*big + 1);
    return p >= sub ? sub : hdr;
  }
  return nullptr;
}

MemBlock SparseHeap::slab_range() const {
  // requires sort() first
  return m_slabs.empty() ? MemBlock{nullptr,0} :
         MemBlock{
           m_slabs.front().ptr,
           safe_cast<size_t>((char*)m_slabs.back().ptr -
                             (char*)m_slabs.front().ptr) + kSlabSize
         };
}

}
