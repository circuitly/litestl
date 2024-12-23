#pragma once

/** Small linear algebra vector class. */

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <type_traits>
#include <utility>

#include "util/type_tags.h"

namespace litestl::math {
template <typename T, int vec_size> class Vec {
public:
  using value_type = T;

  static const int size = vec_size;

  constexpr Vec()
  {
    vec_[0] = vec_[1] = T(0);
  }

  constexpr Vec(T a, T b)
  {
    vec_[0] = a;
    vec_[1] = b;

    for (int i = 2; i < vec_size; i++) {
      vec_[i] = 0.0f;
    }
  }

  constexpr Vec(T a, T b, T c)
  {
    vec_[0] = a;
    vec_[1] = b;
    vec_[2] = c;

    for (int i = 3; i < vec_size; i++) {
      vec_[i] = 0.0f;
    }
  }

  constexpr Vec(T a, T b, T c, T d)
  {
    vec_[0] = a;
    vec_[1] = b;
    vec_[2] = c;
    vec_[3] = d;

    for (int i = 4; i < vec_size; i++) {
      vec_[i] = 0.0f;
    }
  }

  constexpr Vec(T single)
  {
    for (int i = 0; i < vec_size; i++) {
      vec_[i] = single;
    }
  }

  constexpr Vec(const T *value)
  {
    for (int i = 0; i < vec_size; i++) {
      vec_[i] = value[i];
    }
  }

  constexpr Vec(const Vec &b)
  {
    for (int i = 0; i < vec_size; i++) {
      vec_[i] = b.vec_[i];
    }
  }

  constexpr Vec(Vec &b)
  {
    for (int i = 0; i < vec_size; i++) {
      vec_[i] = b.vec_[i];
    }
  }

  constexpr inline Vec &zero()
  {
    for (int i = 0; i < vec_size; i++) {
      vec_[i] = T(0);
    }

    return *this;
  }

  constexpr inline T &operator[](int idx)
  {
    return vec_[idx];
  }

  constexpr inline T operator[](int idx) const
  {
    return vec_[idx];
  }

#ifdef VEC_OP_DEF
#undef VEC_OP_DEF
#endif

#define VEC_OP_DEF(op)                                                                   \
  inline constexpr Vec operator op(const Vec &b) const                                   \
  {                                                                                      \
    Vec r;                                                                               \
    for (int i = 0; i < vec_size; i++) {                                                 \
      r[i] = vec_[i] op b.vec_[i];                                                       \
    }                                                                                    \
    return r;                                                                            \
  }                                                                                      \
  inline constexpr Vec operator op(T b) const                                            \
  {                                                                                      \
    Vec r;                                                                               \
    for (int i = 0; i < vec_size; i++) {                                                 \
      r[i] = vec_[i] op b;                                                               \
    }                                                                                    \
    return r;                                                                            \
  }                                                                                      \
  inline constexpr Vec &operator op##=(T b)                                              \
  {                                                                                      \
    for (int i = 0; i < vec_size; i++) {                                                 \
      vec_[i] op## = b;                                                                  \
    }                                                                                    \
    return *this;                                                                        \
  }                                                                                      \
  inline constexpr Vec &operator op##=(const Vec b)                                      \
  {                                                                                      \
    for (int i = 0; i < vec_size; i++) {                                                 \
      vec_[i] op## = b.vec_[i];                                                          \
    }                                                                                    \
    return *this;                                                                        \
  }

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-token-paste"
#endif

  VEC_OP_DEF(*)
  VEC_OP_DEF(/)
  VEC_OP_DEF(+)
  VEC_OP_DEF(-)

#ifdef __clang__
#pragma clang diagnostic pop
#endif

  constexpr Vec &min(const Vec &b)
  {
    for (int i = 0; i < vec_size; i++) {
      vec_[i] = std::min(vec_[i], b.vec_[i]);
    }

    return *this;
  }

  constexpr Vec &max(const Vec &b)
  {
    for (int i = 0; i < vec_size; i++) {
      vec_[i] = std::max(vec_[i], b.vec_[i]);
    }

    return *this;
  }

  constexpr Vec &floor()
  {
    for (int i = 0; i < vec_size; i++) {
      vec_[i] = std::floor(vec_[i]);
    }

    return *this;
  }

  constexpr Vec &ceil()
  {
    for (int i = 0; i < vec_size; i++) {
      vec_[i] = std::ceil(vec_[i]);
    }

    return *this;
  }

  constexpr Vec &abs()
  {
    for (int i = 0; i < vec_size; i++) {
      vec_[i] = std::abs(vec_[i]);
    }

    return *this;
  }

  constexpr Vec &fract()
  {
    for (int i = 0; i < vec_size; i++) {
      vec_[i] -= std::floor(vec_[i]);
    }

    return *this;
  }

  T dot(const Vec &b)
  {
    T sum = T(0);
    for (int i = 0; i < vec_size; i++) {
      sum += vec_[i] * b.vec_[i];
    }

    return sum;
  }

  constexpr T normalize()
  {
    T len = length();
    if (len > 0.00000001) {
      double mul = 1.0 / double(len);

      for (int i = 0; i < vec_size; i++) {
        vec_[i] = T(double(vec_[i]) * mul);
      }
    } else {
      zero();
      len = 0.0;
    }

    return len;
  }

  constexpr T length()
  {
    return std::sqrt(dot(*this));
  }

  constexpr T lengthSqr()
  {
    return dot(*this);
  }

  constexpr T distance(const Vec &b)
  {
    return std::sqrt(distanceSqr(b));
  }

  constexpr T distanceSqr(const Vec &b)
  {
    return (b - *this).lengthSqr();
  }

  constexpr Vec &rotate2d(Vec center, double th)
  {
    if constexpr (vec_size > 1) {
      double cosTh = std::cos(th);
      double sinTh = std::sin(th);
      double x = vec_[0] - center[0];
      double y = vec_[1] - center[1];

      vec_[0] = cosTh * x - sinTh * y;
      vec_[1] = cosTh * y + sinTh * x;
    }

    return *this;
  }

  constexpr Vec &interp(const Vec &b, double factor)
  {
    for (int i = 0; i < vec_size; i++) {
      vec_[i] += T(double(b.vec_[i] - vec_[i]) * factor);
    }

    return *this;
  }

  operator T *()
  {
    return vec_;
  }

  void print()
  {
    printf("(");
    for (int i = 0; i < vec_size; i++) {
      if (i > 0) {
        printf(" ");
      }
      printf("%.4f", vec_[i]);
    }
    printf(")");
  }

  /* modifies vector in-place */
  constexpr Vec &negate()
  {
    for (int i = 0; i < vec_size; i++) {
      vec_[i] = -vec_[i];
    }
    return *this;
  }

private:
  T vec_[vec_size];
};

#ifdef DEF_VECS
#undef DEF_VECS
#endif
} // namespace litestl::math

#define DEF_VECS(type, name)                                                             \
  namespace litestl::math {                                                              \
  using name##1 = Vec<type, 1>;                                                          \
  using name##2 = Vec<type, 2>;                                                          \
  using name##3 = Vec<type, 3>;                                                          \
  using name##4 = Vec<type, 4>;                                                          \
  }                                                                                      \
  force_type_is_simple(litestl::math::name##1);                                          \
  force_type_is_simple(litestl::math::name##2);                                          \
  force_type_is_simple(litestl::math::name##3);                                          \
  force_type_is_simple(litestl::math::name##4);

DEF_VECS(float, float);
DEF_VECS(double, double);
DEF_VECS(int32_t, int);
DEF_VECS(uint32_t, uint);
DEF_VECS(int16_t, short);
DEF_VECS(uint16_t, ushort);
DEF_VECS(int8_t, char);
DEF_VECS(uint8_t, uchar);

#undef DEF_VECS
