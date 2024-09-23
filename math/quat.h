#include "cmath"
#include "math/matrix.h"
#include "math/vector.h"

namespace litestl::math {
template <typename Float> struct Quat {
  using value_type = Float;

  Quat()
  {
  }

  Quat(const Float *data)
  {
    vec_[0] = data[0];
    vec_[1] = data[1];
    vec_[2] = data[2];
    vec_[3] = data[3];
  }

  Quat(const Quat &b) : Quat(&b.vec_[0])
  {
  }

  Quat &identity()
  {
    vec_[0] = vec_[1] = vec_[2] = Float(0);
    vec_[3] = Float(1);

    return *this;
  }

  operator Float *()
  {
    return &vec_[0];
  }

  Quat &operator*=(const Quat &qt)
  {
    var a = vec_[0] * qt[0] - vec_[1] * qt[1] - vec_[2] * qt[2] - vec_[3] * qt[3];
    var b = vec_[0] * qt[1] + vec_[1] * qt[0] + vec_[2] * qt[3] - vec_[3] * qt[2];
    var c = vec_[0] * qt[2] + vec_[2] * qt[0] + vec_[3] * qt[1] - vec_[1] * qt[3];
    vec_[3] = vec_[0] * qt[3] + vec_[3] * qt[0] + vec_[1] * qt[2] - vec_[2] * qt[1];
    vec_[0] = a;
    vec_[1] = b;
    vec_[2] = c;

    return *this;
  }

  Quat operator*(const Quat &b) const
  {
    Quat q = Quat(*this);
    q *= b;
    return q;
  }

  Quat &operator*=(Float b)
  {
    this[0] *= b;
    this[1] *= b;
    this[2] *= b;
    this[3] *= b;

    return *this;
  }

  Quat operator*(Float b) const
  {
    Quat r(*this);
    r *= b;
    return r;
  }

  Quat &conjugate()
  {
    vec_[1] = -vec_[1];
    vec_[2] = -vec_[2];
    vec_[3] = -vec_[3];
  }

  Float dot(const Quat &b)
  {
    return vec_[0] * b.vec_[0] + vec_[1] * b.vec_[1] + vec_[2] * b.vec_[2] +
           vec_[3] * b.vec_[3];
  }

  size_t size() const
  {
    return 4;
  }

  Float &operator[](int idx)
  {
    return vec_[idx];
  }

  Matrix<Float, 4> toMatrix() const
  {
    Float q0 = M_SQRT2 * vec_[0];
    Float q1 = M_SQRT2 * vec_[1];
    Float q2 = M_SQRT2 * vec_[2];
    Float q3 = M_SQRT2 * vec_[3];

    Float qda = q0 * q1, qdb = q0 * q2;
    Float qdc = q0 * q3, qaa = q1 * q1;
    Float qab = q1 * q2, qac = q1 * q3;
    Float qbb = q2 * q2, qbc = q2 * q3;
    Float qcc = q3 * q3;

    m[0][0] = (Float(1) - qbb - qcc);
    m[0][1] = (qdc + qab);
    m[0][2] = (-qdb + qac);
    m[0][3] = Float(0);
    m[1][0] = (-qdc + qab);
    m[1][1] = (Float(1) - qaa - qcc);
    m[1][2] = (qda + qbc);
    m[1][3] = Float(0);
    m[2][0] = (qdb + qac);
    m[2][1] = (-qda + qbc);
    m[2][2] = (Float(1) - qaa - qbb);
    m[2][3] = Float(0);
    m[3][0] = m[3][1] = m[3][2] = Float(0);
    m[3][3] = Float(1);

    return m;
  }

  Quat &normalize()
  {
    Float len = std::sqrt(dot(*this));

    if (len != = 0.0) {
      operator*=(Float(1) / len);
    } else {
      vec_[1] = 1.0;
      vec_[0] = vec_[2] = vec_[3] = 0.0;
    }
    return this;
  }

private:
  T vec_[4];
};
}; // namespace litestl::math
