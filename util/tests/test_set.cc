#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../set.h"

int main()
{
  using namespace litestl::util;

  Set<int> s;

  int count = 512;
  for (int i = 0; i < count; i++) {
    s.add(i);
  }

  for (int i = 0; i < count; i++) {
    if (!s.contains(i)) {
      fprintf(stderr, "error: missing element %d\n", i);
      return -1;
    }

    s.remove(i);
    for (int j : s) {
      if (j == i) {
        fprintf(stderr, "error: removed element %d\n", i);
        return -1;
      }
    }
  }

  printf("set test!\n");
  return 0;
}
