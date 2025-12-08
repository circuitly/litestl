#include "litestl/math/matrix.h"
#include "test_util.h"
#include "litestl/util/rand.h"
#include "litestl/util/set.h"
#include "litestl/util/string.h"
#include "litestl/util/vector.h"
#include <cstdio>

test_init;

int main()
{
  using namespace litestl::math;
  using namespace litestl::util;

  {
    Random rand;

    mat4 m4;
    mat3 m3;
    float3 v;

    m4.identity();
    m3.identity();

    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        m4[i][j] = rand.get_float() * 2.0 - 1.0;
      }
    }

    v = m4 * float3(1.0f);

    mat4 m5(m4);
    m5.invert();

    printf("det: %.5f\n", m4.determinant());

    m4.print();
    m5.print();

    printf("\n\n");
    mat4 m6 = m4 * m5;
    m6.print();

    mat4 i4;
    i4.identity();

    float dist = i4.dist(m6);
    printf("dist %f\n", dist);

    if (std::fabs(dist) > 0.0001) {
      printf("Matrix inversion error\n");
      return -1;
    }
  }

  return test_end();
}
