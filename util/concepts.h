#pragma once

#include <concepts>
#include <type_traits>
namespace litestl::util {
template <typename T, typename... U> concept IsAnyOf = (std::same_as<T, U> || ...);
template <typename T, typename B> concept SameAsQualified = requires(T a)
{
  std::is_same_v<T, B>;
  std::is_const_v<T> == std::is_const_v<B>;
  std::is_volatile_v<T> == std::is_volatile_v<B>;
  std::is_reference_v<T> == std::is_reference_v<B>;
  std::is_rvalue_reference_v<T> == std::is_rvalue_reference_v<B>;
};
template <typename T> concept NotConst = requires(T a)
{
  !std::is_const_v<T>;
};

}; // namespace litestl::util
