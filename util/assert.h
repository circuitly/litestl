#pragma once

#include <cstdio>

/* TODO: implement assertions properly. */
namespace litestl::util {
static bool assert(bool state, const char *msg = "Assertion failed")
{
  if (!state) {
    fprintf(stderr, "Assertion failed: \"%s\"\n", msg);
    return false;
  }

  return true;
}
}; // namespace litestl::util
