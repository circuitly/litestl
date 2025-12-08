
#include "test_util.h"
#include "litestl/util/index_range.h"
#include "litestl/util/rand.h"
#include "litestl/util/set.h"
#include "litestl/util/string.h"
#include "litestl/util/task.h"
#include "litestl/util/vector.h"
#include <cstdio>

test_init;

int main()
{
  using namespace litestl;
  using namespace litestl::util;

  {
    int size = 1 << 25;

    task::parallel_for(
        IndexRange(size),
        [](IndexRange range) {
          volatile int d = 0;

          for (int i : range) {
            d += i >> 16;
          }

          printf("%d:\n", d);
        },
        1 << 16);
  }
  return test_end();
}
