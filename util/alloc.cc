#include "atomicLinkedList.h"
// #include "compiler_util.h"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <mutex>

#ifdef WASM
#include "wasm.h"
#endif

#define MAKE_TAG(a, b, c, d) (a | (b << 8) | (c << 16) | d << 24)
#define TAG1 MAKE_TAG('t', 'a', 'g', '1')
#define TAG2 ('T' | ('A' << 8) | ('g' << 16))

#define FREE MAKE_TAG('f', 'r', 'e', 'e')

std::atomic<int> memorySize = {0};
std::atomic<int> permanentMemorySize = {
    0}; // size of things allocated when disableLeakTracking > 0
std::atomic<int> allocatingPermanent = {0};

enum { MEM_PERMANENT = 1 << 0 };

struct MemHead {
  int tag1;
  int tag2 : 24;
  int flag : 8;

  // MemHeads are their own nodes
  struct MemHead *next, *prev;
  struct MemHead *data()
  {
    return this;
  }
  static MemHead *wrapData(MemHead *mh)
  {
    return mh;
  }

  size_t size;
  const char *tag;
  void *listref;
#ifdef WASM
  // pad to 8 bytes
  char _pad[4];
#endif
};
static_assert(sizeof(MemHead) % 8 == 0);

using MemHeadPtr = MemHead *;
using MemList = litestl::util::AtomicLinkedList<MemHead, MemHead *>;

/**
 * Each thread creates its own list of memory blocks.
 * We allow these to leak when threads are destroyed
 * (remember the actual memory blocks are kept alive)
 * though this shouldn't happen if you use the
 * `litestil::task` api.
 */
thread_local MemList *atomic_mem_list = nullptr;

static MemList *getMemList()
{
  if (!atomic_mem_list) {
    atomic_mem_list = new MemList();
  }
  return atomic_mem_list;
}
namespace litestl::alloc {
#ifndef NO_DEBUG_ALLOC
bool print_blocks(bool printPermanent)
{
  std::lock_guard guard(getMemList()->mutex);

  MemHead *mem = getMemList()->first;
  while (mem) {
    if (!(mem->flag & MEM_PERMANENT) != printPermanent) {
      printf("\"%s:%d\"  (%p)\n", mem->tag, int(mem->size), mem + 1);
    }
    mem = mem->next;
  }

  return getMemList()->first != nullptr;
}

int getMemorySize()
{
  return memorySize.load();
}

int getPermanentMemorySize()
{
  return permanentMemorySize.load();
}

void *alloc(const char *tag, size_t size)
{
  size_t newsize = size + sizeof(MemHead);
  MemHead *mem = reinterpret_cast<MemHead *>(malloc(newsize));

  if (mem == nullptr) {
    fprintf(stderr, "allocation error of size %d\n", int(size));
    return nullptr;
  }

#if defined(ALLOC_SAVE_STACK_TRACES) && defined(WASM)
  tag = litestl::util::wasm::getStackTrace(tag);
#endif

  mem->tag1 = TAG1;
  mem->tag2 = TAG2;
  mem->tag = tag;
  mem->size = size;
  mem->listref = static_cast<void *>(getMemList());
  mem->flag = 0;

  int permanentMem = allocatingPermanent.load();
  if (permanentMem) {
    mem->flag |= MEM_PERMANENT;
    std::atomic_fetch_add(&permanentMemorySize, mem->size + sizeof(MemHead));
  } else {
    std::atomic_fetch_add(&memorySize, mem->size + sizeof(MemHead));
  }

  getMemList()->push(std::move(mem));

  return reinterpret_cast<void *>(mem + 1);
}

bool check_mem(void *ptr)
{
  if (!ptr) {
    return false;
  }

  if (reinterpret_cast<size_t>(ptr) < 1024) {
    fprintf(stderr, "litestl::alloc::check_mem: invalid pointer\n");
    return false;
  }

  MemHead *mem = static_cast<MemHead *>(ptr);
  mem--;

  if (mem->tag1 == FREE) {
    fprintf(stderr, "litestl::alloc::check_mem: error: double free\n");
    return false;
  } else if (mem->tag1 != TAG1 || mem->tag2 != TAG2) {
    fprintf(stderr, "litestl::alloc::check_mem: error: invalid memory block\n");
    return false;
  }

  return true;
}

void release(void *ptr)
{
  if (!ptr) {
    fprintf(stderr, "Null pointer dereference\n");
    return;
  }

  if (!check_mem(ptr)) {
    return;
  }

  MemHead *mem = static_cast<MemHead *>(ptr);
  mem--;

  bool permanent = mem->flag & MEM_PERMANENT;

  MemList *list = static_cast<MemList *>(mem->listref);

#if 0
  std::lock_guard guard(list->mutex);
  if (mem->next && !check_mem(static_cast<void *>(mem->next + 1))) {
    fprintf(stderr, "corrupted heap\n");
    return;
  }
  if (mem->prev && !check_mem(static_cast<void *>(mem->prev + 1))) {
    fprintf(stderr, "corrupted heap\n");
    return;
  }
#endif

  mem->tag1 = FREE;

  /* Unlink from list. */
  list->remove(mem);

  if (permanent) {
    std::atomic_fetch_sub(&permanentMemorySize, mem->size + sizeof(MemHead));
  } else {
    std::atomic_fetch_sub(&memorySize, mem->size + sizeof(MemHead));
  }
#if defined(ALLOC_SAVE_STACK_TRACES) && defined(WASM)
  free(static_cast<void *>(const_cast<char *>(mem->tag)));
#endif

  free(static_cast<void *>(mem));
}

namespace detail {
const char *getMemoryTag(void *vmem)
{
  if (!check_mem(vmem)) {
    return nullptr;
  }
  MemHead *mem = static_cast<MemHead *>(vmem);
  mem--;
  return mem->tag;
}
} // namespace detail
#endif

void pushPermanentAlloc()
{
  allocatingPermanent.fetch_add(1);
}
void popPermanentAlloc()
{
  allocatingPermanent.fetch_sub(1);
}
} // namespace litestl::alloc