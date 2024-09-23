#include "compiler_util.h"
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>
#include <memory>

#define MAKE_TAG(a, b, c, d) (a | (b << 8) | (c << 16) | d << 24)
#define TAG1 MAKE_TAG('t', 'a', 'g', '1')
#define TAG2 MAKE_TAG('t', 'a', 'g', '2')
#define FREE MAKE_TAG('f', 'r', 'e', 'e')

int memorySize = 0;
struct MemList;

struct MemHead {
  int tag1, tag2;
  struct MemHead *next, *prev;
  size_t size;
  const char *tag;
  struct MemList *list;
  #ifdef WASM
  // pad to 8 bytes
  char _pad[4];
  #endif
};
static_assert(sizeof(MemHead) % 8 == 0);

struct MemList {
  MemHead *first, *last;
};

std::mutex mem_list_mutex;
thread_local MemList mem_list;

namespace litestl::alloc {
#ifndef NO_DEBUG_ALLOC
bool print_blocks()
{
  MemHead *mem = mem_list.first;
  while (mem) {
    printf("\"%s:%d\"  (%p)\n", mem->tag, int(mem->size), mem + 1);
    mem = mem->next;
  }

  return mem_list.first;
}

int getMemorySize() {
  return memorySize;
}

void *alloc(const char *tag, size_t size)
{
  size_t newsize = size + sizeof(MemHead);
  MemHead *mem = reinterpret_cast<MemHead *>(malloc(newsize));

  if (mem == nullptr) {
    fprintf(stderr, "allocation error of size %d\n", int(size));
    return nullptr;
  }

  mem->tag1 = TAG1;
  mem->tag2 = TAG2;
  mem->tag = tag;
  mem->size = size;

  mem->next = mem->prev = nullptr;

  std::lock_guard guard(mem_list_mutex);

  if (!mem_list.first) {
    mem_list.first = mem_list.last = mem;
  } else {
    mem_list.last->next = mem;
    mem->prev = mem_list.last;
    mem_list.last = mem;
  }

  memorySize += mem->size + sizeof(MemHead);

  return reinterpret_cast<void *>(mem + 1);
}

bool check_mem(void *ptr)
{
  if (!ptr) {
    return false;
  }

  MemHead *mem = static_cast<MemHead *>(ptr);
  mem--;

  if (mem->tag1 == FREE) {
    fprintf(stderr, "litestl::alloc::release: error: double free\n");
    return false;
  } else if (mem->tag1 != TAG1 || mem->tag2 != TAG2) {
    fprintf(stderr, "litestl::alloc::release: error: invalid memory block\n");
    return false;
  }

  return true;
}

void release(void *ptr)
{
  if (!ptr) {
    return;
  }

  if (!check_mem(ptr)) {
    return;
  }

  MemHead *mem = static_cast<MemHead *>(ptr);
  mem--;

  mem->tag1 = FREE;

  std::lock_guard guard(mem_list_mutex);
  
  memorySize -= mem->size + sizeof(MemHead);

  /* Unlink from list. */
  if (mem_list.last == mem) {
    mem_list.last = mem->prev;
    if (mem_list.last) {
      mem_list.last->next = nullptr;
    }
  } else {
    mem->next->prev = mem->prev;
  }

  if (mem_list.first == mem) {
    mem_list.first = mem->next;
    if (mem_list.first) {
      mem_list.first->prev = nullptr;
    }
  } else {
    mem->prev->next = mem->next;
  }

  free(static_cast<void*>(mem));
}

namespace detail {
const char *getMemoryTag(void *vmem) {
  if (!check_mem(vmem)) {
    return nullptr;
  }
  MemHead *mem = static_cast<MemHead*>(vmem);
  mem--;
  return mem->tag;
}
}
#endif
} // namespace litestl::alloc