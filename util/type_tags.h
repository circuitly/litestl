#pragma once

#include <type_traits>

namespace litestl::util::detail {
template <typename T> struct is_simple_override : std::false_type {
};

#define force_type_is_simple(Type)                                                       \
  template <>                                                                            \
  struct litestl::util::detail::is_simple_override<Type> : public std::true_type {    \
  }
} // namespace litestl::util::detail
