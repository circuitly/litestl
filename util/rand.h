#pragma once

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <utility>

/* This is a very bad pseudo-random generator. TODO: use mersenne. */

namespace litestl::util {
class Random {
public:
  Random(uint32_t seed = 0) : seed_(seed)
  {
  }

  Random(const Random &b) : seed_(b.seed_)
  {
  }

  float get_float()
  {
    return float(get_double());
  }

  double get_double()
  {
    return double(get_int()) / double(Random::max_value);
  }

  uint32_t get_int()
  {
    seed_ = (seed_ * 12345 + 234243241) & Random::max_value;
    return seed_;
  }

  static const uint32_t max_value = ((1 << 19) - 1);

private:
  uint32_t seed_;
};
} // namespace litestl::util