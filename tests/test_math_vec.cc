
#include "litestl/math/vector.h"
#include "test_util.h"
#include <cstdio>
#include <cstring>

#include "litestl/util/compiler_util.h"
test_init;

int main()
{
  using namespace litestl;
  using namespace litestl::math;

  static_assert(litestl::util::is_simple<litestl::math::float3>());

#if 1
  {
    float3 vec1(1.0), vec2(2.0);

    vec1 -= vec2;

    vec1.print();
    printf(" ");
    vec2.print();
    printf("\n");
  }
#endif

  return test_end();
}
