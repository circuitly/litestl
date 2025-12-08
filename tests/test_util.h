#pragma once

#include "litestl/util/alloc.h"
#include "litestl/util/string.h"

#include <cstdio>

#define test_init int retval = 0

#define test_assert(expr)                                                                \
  (!(expr) ? (retval = 0, fprintf(stderr, "%s failed\n", #expr), fflush(stderr)) : 0)

extern int retval;
static bool test_end()
{
  return retval || litestl::alloc::print_blocks();
}

static bool test_snapshot(const char *name, const char *data, bool update_snapshot)
{
  using namespace litestl::util;

  char path[2048];
  string old = "no snapshot data";
  sprintf(path, "./%s_snapshot.json", name);

  FILE *file = fopen(path, "rb");
  bool ok;

  int len = strlen(data);

  if (file) {
    ok = true;
    for (int i = 0; i < len; i++) {
      if (feof(file)) {
        ok = false;
        break;
      }

      char c = fgetc(file);
      old += c;
      if (c != data[i]) {
        ok = false;
      }
    }
    fclose(file);
  }

  if (!update_snapshot) {
    fprintf(stderr, "==== New ====\n%s\n=== Old ===\n%s\n\n", data, old.c_str());
    fprintf(stderr, "Snapshot %s failed\n", name);
    return false;
  }
  printf("writing %s", path);
  file = fopen(path, "wb");
  if (!file) {
    fprintf(stderr, "failed to write snapshot %s\n", name);
    return false;
  }

  fwrite(data, len, 1, file);
  fclose(file);
  return true;
}
