#pragma once

using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;

#include <cstdint>
#include <type_traits>
#include <utility>

#include "type_tags.h"

#define DEFAULT_MOVE_ASSIGNMENT(Type)                                                    \
  Type &operator=(Type &&b) noexcept                                                     \
  {                                                                                      \
    if (this == &b) {                                                                    \
      return *this;                                                                      \
    }                                                                                    \
    this->~Type();                                                                       \
    new (static_cast<void *>(this)) Type(std::forward<Type>(b));                         \
    return *this;                                                                        \
  }

static inline void *pointer_offset(void *ptr, int n)
{
  return static_cast<void *>(static_cast<char *>(ptr) + n);
}

static inline const void *pointer_offset(const void *ptr, int n)
{
  return static_cast<const void *>(static_cast<const char *>(ptr) + n);
}

#define array_size(Array) (sizeof(Array) / sizeof(*(Array)))

#ifdef __clang__
#define ATTR_NO_OPT __attribute__((optnone))
#elif defined(_MSC_VER)
#define ATTR_NO_OPT __pragma(optimize("", off))
#elif defined(__GNUC__)
#define ATTR_NO_OPT __attribute__((optimize("O0")))
#else
#define ATTR_NO_OPT
#endif

#if defined(MSVC) && !defined(__clang__)
#define flatten_inline [[msvc::flatten]]
#define force_inline [[forceinline]]
#elif defined(__clang__)
#define flatten_inline [[gnu::flatten]]
#define force_inline [[clang::always_inline]]
#else
#define flatten_inline [[gnu::flatten]]
#define force_inline [[gnu::always_inline]]
#endif

#define FlagOperators(T)                                                                 \
  static constexpr T operator|(T a, T b)                                                 \
  {                                                                                      \
    return T(uint64_t(a) | uint64_t(b));                                                 \
  }                                                                                      \
  static constexpr T operator&(T a, T b)                                                 \
  {                                                                                      \
    return T(uint64_t(a) & uint64_t(b));                                                 \
  }                                                                                      \
  static constexpr T operator~(T a)                                                      \
  {                                                                                      \
    return T(~uint64_t(a));                                                              \
  }                                                                                      \
  static constexpr T operator^(T a, T b)                                                 \
  {                                                                                      \
    return T(uint64_t(a) ^ uint64_t(b));                                                 \
  }                                                                                      \
  static T operator^=(T a, T flag)                                                       \
  {                                                                                      \
    return T(uint64_t(a) ^ uint64_t(flag));                                              \
  }                                                                                      \
  static T operator|=(T a, T flag)                                                       \
  {                                                                                      \
    return T(uint64_t(a) | uint64_t(flag));                                              \
  }                                                                                      \
  static T operator&=(T a, T flag)                                                       \
  {                                                                                      \
    return T(uint64_t(a) & uint64_t(flag));                                              \
  }

/* Example:
    enum class _AttrFlag {
      NONE = 0,
      TOPO = 1 << 0,
      TEMP = 1 << 1,
      NOCOPY = 1 << 2,
      NOINTERP = 1 << 3,
    };
    FlagClass(AttrFlag, _AttrFlag);
 */
#define FlagClass(Name, Enum)                                                            \
  struct Name {                                                                          \
    using enum Enum;                                                                     \
    constexpr Name() : val_(Enum(0))                                                     \
    {                                                                                    \
    }                                                                                    \
    constexpr Name(Enum f) : val_(f)                                                     \
    {                                                                                    \
    }                                                                                    \
    constexpr Name(int val) : val_(static_cast<Enum>(val))                               \
    {                                                                                    \
    }                                                                                    \
    constexpr Name(const Name &b) : val_(b.val_)                                         \
    {                                                                                    \
    }                                                                                    \
    constexpr operator bool() const                                                      \
    {                                                                                    \
      return bool(int(val_));                                                            \
    }                                                                                    \
    constexpr explicit operator int() const                                              \
    {                                                                                    \
      return int(val_);                                                                  \
    }                                                                                    \
    constexpr bool operator==(Name b) const                                              \
    {                                                                                    \
      return val_ == b.val_;                                                             \
    }                                                                                    \
    constexpr bool operator!=(Name b) const                                              \
    {                                                                                    \
      return val_ != b.val_;                                                             \
    }                                                                                    \
    constexpr Name operator&(Name b) const                                               \
    {                                                                                    \
      return Name(int(val_) & int(b.val_));                                              \
    }                                                                                    \
    constexpr Name operator|(Name b) const                                               \
    {                                                                                    \
      return Name(int(val_) | int(b.val_));                                              \
    }                                                                                    \
    constexpr Name operator^(Name b) const                                               \
    {                                                                                    \
      return Name(int(val_) ^ int(b.val_));                                              \
    }                                                                                    \
    constexpr Name operator~() const                                                     \
    {                                                                                    \
      return Name(~int(val_));                                                           \
    }                                                                                    \
    Enum val_;                                                                           \
  }

namespace litestl::util {
namespace detail {
template <typename T> static constexpr bool is_simple(T *)
{
  return std::is_integral_v<T> || std::is_pointer_v<T> || std::is_floating_point_v<T> ||
         is_simple_override<T>::value;
}
} // namespace detail

template <typename T> static constexpr bool is_simple()
{
  return detail::is_simple(static_cast<std::remove_cv_t<T> *>(nullptr));
}

} // namespace litestl::util
