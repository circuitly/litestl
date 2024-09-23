#include "math/vector.h"
#include <memory>

namespace litestl::math {
template <typename Float = float, int Len, double EPS=1e-18> struct LUT {
  Float[Len] data;

  struct {
    Float min;
    Float max;
    Float invMaxMin;
  } input;

  struct {
    Float min;
    Float max;
  } output;

  Float evaluate(Float t)
  {
    t = (t - input.min) * input.invMaxMin;

    if (t < EPS) {
      t = 0.0;
    } else if (t >= 1.0 - EPS) {
      t = 1.0;
    }

    t *= Len - 1;
    int i1 = int(t);
    int i2 = i1 + 1;

    if (i2 >= Len) {
      i2 = Len - 1;
    }

    float a = data[i1];
    float b = data[i2];
    
    t -= float(i1);
    return a + (b - a) * t;
  }
};

using LUT255f = LUT<float, 255>;
using LUT255fRef = std::shared_ptr<LUT255f>;

} // namespace litestl::math