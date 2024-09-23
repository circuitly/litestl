#pragma once

#include "eigen/include/eigen3/Eigen/Core"
#include "eigen/include/eigen3/Eigen/LU"
#include "math/vector.h"

#include <cstdio>

namespace litestl::math {
template <typename Float, int N, int Options=Eigen::ColMajor> struct Matrix {
  using Vector = Vec<Float, N>;
  using Vec3 = Vec<Float, 3>;
  using Vec4 = Vec<Float, 4>;
  using EigenMatrix = Eigen::Matrix<Float, N, N, Options>;

  Matrix()
  {
  }

  Matrix(const Float *data)
  {
    Float *mat = &mat_[0][0];
    for (int i = 0; i < N * N; i++, mat++, data++) {
      *mat = *data;
    }
  }

  Matrix(const Matrix &b)
  {
    for (int i = 0; i < N; i++) {
      mat_[i] = b.mat_[i];
    }
  }

  Matrix(const EigenMatrix &b)
  {
    /* Use assignment operator. */
    *this = b;
  }

  operator Float *() const
  {
    return &mat_[0][0];
  }

  inline Matrix &identity()
  {
    zero();

    for (int i = 0; i < N; i++) {
      mat_[i][i] = 1.0f;
    }
    return *this;
  }

  static inline constexpr Matrix Identity()
  {
    Matrix r;
    r.identity();
    return r;
  }

  inline Matrix &zero()
  {
    for (int i = 0; i < N; i++) {
      mat_[i].zero();
    }

    return *this;
  }

  Matrix &transpose()
  {
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < i; j++) {
        std::swap(mat_[i][j], mat_[j][i]);
      }
    }

    return *this;
  }

  Matrix &invert()
  {
#if 0
    EigenMatrix *m = reinterpret_cast < EigenMatrix *>(this);
    *m = m->inverse();
    return *this;
#else
    EigenMatrix m(&mat_[0][0]);
    EigenMatrix r = m.inverse();
    *this = r;
#endif

    return *this;
  }

  /* Vector multiplication. */
  template <int M> inline Vec<Float, M> operator*(const Vec<Float, M> v) const
  {
    Vec<Float, M> r;

    constexpr int n = N < M ? N : M;

    for (int i = 0; i < n; i++) {
      Float sum = Float(0);

      for (int j = 0; j < n; j++) {
        sum += v[j] * mat_[j][i];
      }

      if constexpr (N == 4 && M < 4) {
        sum += mat_[3][i];
      }

      r[i] = sum;
    }

    return r;
  }

  /* Matrix multiplication. */
  Matrix operator*(const Matrix &b) const
  {
    Matrix r;

    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        Float sum = Float(0);

        for (int k = 0; k < N; k++) {
          sum += mat_[i][k] * b.mat_[k][j];
        }

        r.mat_[i][j] = sum;
      }
    }

    return r;
  }

  Float determinant() const
  {
    const EigenMatrix *mat = reinterpret_cast<const EigenMatrix *>(this);

    return mat->determinant();
  }

  Float dist(const Matrix &b) const
  {
    Matrix r = *this;
    Float sum = Float(0);

    for (int i = 0; i < N; i++) {
      r.mat_[i] -= b.mat_[i];
      sum += r[i].dot(r[i]);
    }

    return std::sqrt(sum);
  }

  Matrix &operator*=(const Matrix &b)
  {
    Matrix r = *this * b;
    *this = r;
    return r;
  }

  Matrix &operator=(const Matrix &b)
  {
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        mat_[i][j] = b.mat_[i][j];
      }
    }

    return *this;
  }

  operator EigenMatrix() const
  {
    return EigenMatrix(&mat_[0][0]);
  }

  Matrix &operator=(const EigenMatrix &b)
  {
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        mat_[i][j] = b(i, j);
      }
    }

    return *this;
  }

  Vector &operator[](int idx)
  {
    return mat_[idx];
  }

  int size()
  {
    return N;
  }

  void print(int dec = 2)
  {
    char fmt[32];

    sprintf(fmt, "%%.%df ", dec);

    auto wid = [](double v) {
      if (std::fabs(v) <= 1.0) {
        return 1;
      }

      v += 0.25;

      double size = ceil(log(std::fabs(v)) / log(10.0));
      return int(size);
    };

    int maxwid = 0;
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        maxwid = std::max(maxwid, wid(mat_[i][j]));
      }
    }

    printf("\n");
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        int n = maxwid - wid(mat_[i][j]);

        printf(fmt, mat_[i][j]);

        for (int k = 0; k < n; k++) {
          printf(" ");
        }
      }

      printf("\n");
    }
  }

private:
  Vector mat_[N];
};

using mat3 = Matrix<float, 3>;
using mat4 = Matrix<float, 4>;

} // namespace litestl::math
