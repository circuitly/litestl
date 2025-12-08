#include "test_util.h"
#include "litestl/util/alloc.h"
#include "litestl/util/rand.h"
#include "litestl/util/set.h"
#include "litestl/util/string.h"
#include "litestl/util/vector.h"
#include <cstdio>

#include "litestl/util/memory.h"

test_init;

struct Test {
  Test()
  {
    printf("Allocate Test\n");
  }

  ~Test()
  {
    printf("Deallocate Test\n");
  }
};

int main()
{
  using namespace litestl;
  using litestl::util::shared_ptr;
  using litestl::util::weak_ptr;

  weak_ptr<Test> weak;

  {
    Test *test = alloc::New<Test>("Test");

    shared_ptr<Test> ptr{test};
    shared_ptr<Test> ptr2 = ptr;

    weak = ptr2;

    printf("exists1: %d\n", int(!weak.expired()));
    test_assert(!weak.expired());
  }

  test_assert(weak.expired());
  printf("exists2: %d\n", int(!weak.expired()));

  return test_end();
}
