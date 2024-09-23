#pragma once

#include <algorithm>
#include <compare>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>

#include "util/alloc.h"
#include "util/compiler_util.h"

namespace litestl::util {
template <typename Char, int static_size = 32> struct String;

template <size_t N> struct StrLiteral {
  constexpr StrLiteral(const char (&str)[N])
  {
    std::copy_n(str, N, value);
    value[N] = 0;
  }

  char value[N + 1];
};

template <typename Char, int static_size = 32> struct ConstStr {
  constexpr ConstStr()
  {
    size_ = 0;
    zero_data();
  }

  constexpr ConstStr(const ConstStr &b)
  {
    for (int i = 0; i < static_size; i++) {
      data_[i] = b.data_[i];
    }

    data_[b.size_] = 0;
    size_ = b.size_;
  }

  constexpr ConstStr(const char *str)
  {
    zero_data();

    const char *c = str;
    while (*c && size_ < static_size - 1) {
      data_[size_++] = *c;
      c++;
    }
  }

  template <size_t N> constexpr ConstStr(StrLiteral<N> lit)
  {
    size_ = N - 1 > static_size - 1 ? static_size - 1 : N - 1;

    for (int i = 0; i < size_; i++) {
      data_[i] = lit.value[i];
    }
    data_[size_] = 0;
  }

  constexpr bool operator==(const ConstStr &b)
  {
    if (size_ != b.size_) {
      return false;
    }

    for (int i = 0; i < size_; i++) {
      if (data_[i] != b.data_[i]) {
        return false;
      }
    }

    return true;
  }

  constexpr bool operator!=(const ConstStr &b)
  {
    return !operator==(b);
  }

  constexpr size_t size()
  {
    return size_;
  }

  constexpr Char operator[](int idx)
  {
    return data_[idx];
  }

private:
  constexpr void zero_data()
  {
    for (int i = 0; i < static_size; i++) {
      data_[i] = 0;
    }
  }

  Char data_[static_size];
  int size_ = 0;
};

using const_string = ConstStr<char>;

namespace detail {
template <typename Char> int strcmp(const Char *a, const Char *b)
{
  if (!a || !b) {
    return false;
  }

  while (*a && *b) {
    if (*a < *b) {
      return -1;
    } else if (*a > *b) {
      return 1;
    }

    a++;
    b++;
  }

  if (*a || *b) {
    return *a ? -1 : 1;
  }

  return 0;
}
} // namespace detail

template <typename Char> struct StringRef {
  StringRef()
  {
  }
  StringRef(const char *c) : data_(c), size_(strlen(c))
  {
  }
  StringRef(const StringRef &b) : data_(b.data_), size_(b.size_)
  {
  }

  operator String<Char>()
  {
    return String<Char>(data_);
  }

  bool operator!=(const StringRef &vb) const
  {
    return !operator==(vb);
  }

  bool operator==(const StringRef &vb) const
  {
    if (size_ != vb.size_) {
      return false;
    }

    for (int i = 0; i < size_; i++) {
      if (this->data_[i] != vb.data_[i]) {
        return false;
      }
    }

    return true;
  }

  using const_char_star = const char *;

  operator const_char_star() const
  {
    return data_;
  }

  inline const char *c_str() const
  {
    return data_;
  }

  inline const char operator[](int idx) const
  {
    return data_[idx];
  }

  inline size_t size() const
  {
    return size_;
  }

  bool operator==(StringRef &b) const
  {
    return detail::strcmp(data_, b.data_);
  }

  bool operator!=(StringRef &b) const
  {
    return !operator==(b);
  }

private:
  const char *data_ = nullptr;
  int size_ = 0;
};

template <typename Char, int static_size> class String {
public:
  String() : size_(0)
  {
    data_ = static_storage_;
    data_[0] = 0;
  }

  operator StringRef<Char>() const
  {
    return StringRef<Char>(data_);
  }

  template <size_t N> String(StrLiteral<N> lit)
  {
    data_ = static_storage_;
    ensure_size(N);
    size_ = N;

    for (int i = 0; i < N; i++) {
      data_[i] = lit.value[i];
    }
    data_[N] = 0;
  }

  ATTR_NO_OPT String(const String &b) : size_(static_size - 1)
  {
    data_ = static_storage_;

    if (b.data_) {
      ensure_size(b.size_);
      size_ = b.size_;
      // std::copy_n(b.data_, size_, data_);
      for (int i = 0; i < b.size_; i++) {
        data_[i] = b.data_[i];
      }
      data_[size_] = 0;
    } else {
      size_ = 0;
      data_[0] = 0;
    }
  }

  String(String &&b)
  {
    if (b.data_ == b.static_storage_) {
      data_ = static_storage_;

      for (int i = 0; i < b.size_; i++) {
        data_[i] = b.data_[i];
      }

      data_[b.size_] = 0;
    } else {
      data_ = b.data_;
      b.data_ = b.static_storage_;
      b.data_[0] = 0;
    }

    size_ = b.size_;
  }

  String &operator=(const String &b)
  {
    data_ = static_storage_;
    ensure_size(size_);

    for (int i = 0; i < b.size_; i++) {
      data_[i] = b.data_[i];
    }

    size_ = b.size_;
    data_[size_] = 0;

    return *this;
  }

  inline Char operator[](int idx)
  {
    return data_[idx];
  }

  DEFAULT_MOVE_ASSIGNMENT(String)

  ~String()
  {
    if (data_ && data_ != static_storage_) {
      alloc::release(static_cast<void *>(data_));
    }
  }

  String(const char *str)
  {
    data_ = static_storage_;

    int len = strlen(str);

    ensure_size(len);
    size_ = len;

    for (int i = 0; i < len; i++) {
      data_[i] = str[i];
    }
    data_[len] = 0;
  }

  String(const StringRef<Char> &ref)
  {
    String(ref.c_str());
  }

  const char *c_str() const
  {
    return data_;
  }

  operator const char *() const
  {
    return data_;
  }

  bool operator!=(const String &vb) const
  {
    return !operator==(vb);
  }

  bool operator==(const String &vb) const
  {
    return detail::strcmp<Char>(c_str(), vb.c_str()) == 0;
  }

  String operator+(const String &b) const
  {
    return String(*this).operator+=(b);
  }

  String &operator+=(const String &b)
  {
    ensure_size(size_ + b.size_);

    int j = size_;
    for (int i = 0; i < b.size_; i++) {
      data_[j++] = b.data_[i];
    }

    size_ += b.size_;
    data_[size_] = 0;

    return *this;
  }

  size_t size() const
  {
    return size_;
  }

private:
  /* Ensures data has at least size+1 elements, does not set size_*/
  void ensure_size(int size)
  {
    if (size >= size_ + 1) {
      Char *data2;

      if (size + 1 <= static_size) {
        data2 = static_storage_;
      } else {
        data2 = static_cast<Char *>(alloc::alloc("string", size + 1));
      }

      for (int i = 0; i < size_; i++) {
        data2[i] = data_[i];
      }

      if (data_ != static_storage_) {
        alloc::release(static_cast<void *>(data_));
      }
      data_ = data2;
    }
  }

  Char *data_;
  Char static_storage_[static_size];
  int size_ = 0; /* does not include null-terminating byte. */
};

template <typename Char> String<Char> operator+(const Char *a, const String<Char> &b)
{
  return String<Char>(a) + b;
}

using string = String<char>;
using stringref = StringRef<char>;

using StringKey = int;

/* Get a unique integer key for str. */
StringKey get_stringkey(const stringref str);

} // namespace litestl::util
