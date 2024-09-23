#pragma once 
#include "alloc.h"
#include "compiler_util.h"
#include "vector.h"

namespace litestl::util {
template <typename T> struct Array {
  using value_type = T;
  struct iterator {
    inline iterator(Array &array, int i) : array_(array), i_(i)
    {
    }

    inline iterator(const iterator &b) : array_(b.array_), i_(b.i_)
    {
    }

    inline bool operator==(const iterator &b)
    {
      return b.i_ == i_;
    }

    inline bool operator!=(const iterator &b)
    {
      return !(operator==(b));
    }

    inline T &operator*()
    {
      return array_[i_];
    }

    inline const T &operator*() const
    {
      return array_[i_];
    }

    inline iterator &operator++()
    {
      i_++;
      return *this;
    }

  private:
    Array &array_;
    int i_;
  };

  ~Array()
  {
    if (data_) {
      if constexpr (!is_simple<T>()) {
        alloc::DeleteArray<T>(data_, size_);
      } else {
        alloc::release(static_cast<void *>(data_));
      }
    }
  }

  Array() : data_(nullptr), size_(0)
  {
  }

  Array(size_t size) : size_(0), data_(nullptr)
  {
    resize(size);
  }

  Array(T *data, size_t size) : data_(data), size_(size)
  {
  }

  Array(const Array &b) : size_(b.size_)
  {
    data_ = static_cast<T *>(alloc::alloc("Array", size_ * sizeof(T)));

    for (int i = 0; i < size_; i++) {
      data_[i] = b.data_[i];
    }
  }

  Array(Array &&b) : size_(b.size_)
  {
    data_ = std::move(b.data_);

    b.data_ = nullptr;
    b.size_ = 0;
  }

  inline iterator begin()
  {
    return iterator(*this, 0);
  }

  inline iterator end()
  {
    return iterator(*this, size_);
  }

  template <bool construct_destruct = true> void resize(size_t newsize)
  {
    size_t oldsize = size_;

    T *newdata = static_cast<T *>(alloc::alloc(__func__, sizeof(T) * newsize));

    int count = std::min(size_, newsize);

    for (int i = 0; i < count; i++) {
      newdata[i] = std::move(data_[i]);
    }

    if (construct_destruct) {
      if (!is_simple<T>()) {
        /* New size bigger? */
        for (int i = size_; i < newsize; i++) {
          new (static_cast<void *>(&newdata[i])) T();
        }

        /* New size smaller? */
        for (int i = newsize; i < size_; i++) {
          data_[i].~T();
        }
      } else {
        if (size_ < newsize) {
          for (int i = size_; i < newsize; i++) {
            newdata[size_] = T(0);
          }
        }
      }
    }

    alloc::release(static_cast<void *>(data_));

    data_ = newdata;
    size_ = newsize;
  }

  inline T &operator[](int idx)
  {
    return data_[idx];
  }

  inline const T &operator[](int idx) const
  {
    return data_[idx];
  }

  inline size_t size() const
  {
    return size_;
  }

private:
  T *data_;
  size_t size_;
};
} // namespace litestl::util
