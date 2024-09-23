#pragma once

#include <utility>
#include <cstddef>

/*
 * Leak debugger allocator.
 *
 * We don't use operator new or delete, instead
 * we have alloc::New and allow::Delete.  These
 * take a string tag identifying the allocated object
 *
 * All allocated objects (well, memory blocks) are stored in a linked list,
 * which is used to identify leaks.  This is done by calling alloc::print_blocks,
 * typically on applicated exist after everything has been deallocated.
 */
namespace litestl::alloc {

#ifndef NO_DEBUG_ALLOC
void *alloc(const char *tag, size_t size);
void release(void *mem);
bool print_blocks();
int getMemorySize();

namespace detail {
const char *getMemoryTag(void *vmem);
}
template<typename T> static const char *getMemoryTag(T *mem) {
  return detail::getMemoryTag(static_cast<void*>(mem));
}

#else
static int getMemorySize() {
  return -1;
}
static void *alloc(const char *tag, size_t size)
{
  return malloc(size);
}
static void release(void *mem) {
  free(mem);
}
template<typename T> static const char *getMemoryTag(T *mem) {
  return nullptr;
}

static bool print_blocks() {
}
#endif

template <typename T, typename... Args> inline T *New(const char *tag, Args... args)
{
  void *mem = alloc(tag, sizeof(T));

  return new (mem) T(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
inline T *NewArray(const char *tag, size_t size, Args... args)
{
  if (size == 0) {
    return nullptr;
  }

  void *mem = alloc(tag, sizeof(T) * size);
  T *elem = static_cast<T *>(mem);

  for (int i = 0; i < size; i++) {
    new (elem) T(std::forward<Args>(args)...);
  }

  return static_cast<T *>(elem);
}

template <typename T> inline void Delete(T *arg)
{
  if (arg) {
    arg->~T();
    release(static_cast<void *>(arg));
  }
}

template <typename T> inline void DeleteArray(T *arg, size_t size)
{
  if (arg) {
    for (int i = 0; i < size; i++) {
      arg[i].~T();
    }

    release(static_cast<void *>(arg));
  }
}

}; // namespace litestl::alloc
