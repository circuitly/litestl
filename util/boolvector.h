#pragma once

#include "alloc.h"
#include "compiler_util.h"
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace litestl::util {
template <int static_size = 32> class BoolVector {
  using BlockInt = uint32_t;
  static constexpr int block_size = 32;
  static constexpr int block_shift = 4;
  static constexpr int block_mask = 31;
  static constexpr int block_bytes = 4;

public:
  BoolVector()
  {
    vector_ = static_storage_;
    size_ = static_size;
    vector_size_ = static_size >> block_shift;

    for (int i = 0; i < vector_size_; i++) {
      vector_[i] = 0;
    }
  }

  BoolVector(const BoolVector &b)
  {
    resize(b.size_);

    int ilen = std::min(vector_size_, b.vector_size_);

    for (int i = 0; i < ilen; i++) {
      vector_[i] = b.vector_[i];
    }
  }

  BoolVector(BoolVector &&b)
  {
    vector_size_ = b.vector_size_;
    size_ = b.size_;
    used_ = b.used_;

    if (b.vector_ == b.static_storage_) {
      vector_ = static_storage_;

      for (int i = 0; i < vector_size_; i++) {
        vector_[i] = b.vector_[i];
      }
    } else {
      vector_ = b.vector_;
    }

    b.vector_ = nullptr;
    b.size_ = b.vector_size_ = b.used_ = 0;
  }

  DEFAULT_MOVE_ASSIGNMENT(BoolVector)

  BoolVector &operator=(const BoolVector &b)
  {
    if (this == &b) {
      return *this;
    }

    this->~BoolVector();
    new (static_cast<void *>(this)) BoolVector(b);

    return *this;
  }

  ~BoolVector()
  {
    if (vector_ && vector_ != static_storage_) {
      alloc::release(static_cast<void *>(vector_));
    }
  }

  void reset()
  {
    used_ = 0;
  }

  void clear()
  {
    for (int i = 0; i < vector_size_; i++) {
      vector_[i] = BlockInt(0);
    }
  }

  const bool operator[](int index) const
  {
    int bit = 1 << (index & block_mask);
    return vector_[index >> block_shift] & bit;
  }

  void set(int index, bool val)
  {
    int bit = 1 << (index & block_mask);

    if (val) {
      vector_[index >> block_shift] |= bit;
    } else {
      vector_[index >> block_shift] &= ~bit;
    }
  }

  int size()
  {
    return used_;
  }

  void resize(int newsize)
  {
    if (newsize <= size_) {
      used_ = newsize;
      return;
    }

    realloc((newsize * 2) >> block_shift);
  }

  void append(bool val)
  {
    resize(used_ + 1);
    set(used_ - 1, val);
  }

private:
  BlockInt *vector_ = nullptr;
  BlockInt static_storage_[static_size >> block_shift];
  int size_ = 0, vector_size_ = 0;
  int used_ = 0;

  void realloc(int new_vec_size)
  {
    BlockInt *old = vector_;

    vector_ = static_cast<BlockInt *>(
        alloc::alloc("BitVector data", sizeof(BlockInt) * new_vec_size));

    int i = 0;
    for (i = 0; i < vector_size_; i++) {
      vector_[i] = old[i];
    }
    for (; i < new_vec_size; i++) {
      vector_[i] = 0;
    }

    vector_size_ = new_vec_size;
    size_ = new_vec_size << block_shift;

    if (old != static_storage_) {
      alloc::release(static_cast<void *>(old));
    }
  }
};
} // namespace litestl::util
