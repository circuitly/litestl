#pragma once

#include "compiler_util.h"
#include "string.h"

namespace litestl::hash {
using HashInt = uint64_t;

template <typename T> inline HashInt hash(T *ptr)
{
  HashInt h = reinterpret_cast<HashInt>(ptr);

  /* Pointers only use the bottom 48 bits. */
  if constexpr (sizeof(void *) == 8) {
#if 0 // Paranoia check: add final 16 bits to front
    return (h << 16) | (h >> 48);
#else
    return h << 16;
#endif
  } else {
    return h;
  }
}

inline HashInt hash(int i)
{
  return HashInt(i);
}

/** NOT cryptographically secure! */
inline HashInt hash(const char *str)
{
  const char *c = str;
  HashInt h = 0;

  /* TODO: check this. */
  for (; *c; c++) {
    h = ((h + *c) * (*c) + 23423432) & ((1 << 19) - 1);
  }

  return h;
}
inline HashInt hash(const util::string &str)
{
  return hash(str.c_str());
}
inline HashInt hash(const util::stringref &str)
{
  return hash(str.c_str());
}
} // namespace litestl::hash
