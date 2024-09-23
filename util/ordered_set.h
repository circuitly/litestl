#include "boolvector.h"
#include "compiler_util.h"
#include "map.h"
#include "vector.h"

#include <cstdint>

namespace litestl::util {
template <typename Key, int static_size = 4> struct OrderedSet {
  using key_type = Key;

  struct iterator {
    iterator(const OrderedSet *set, int i) : set_(set), i_(i)
    {
      /* Find first element. */
      if (i == 0) {
        i_--;
        operator++();
      }
    }

    iterator(const iterator &b) : set_(b.set_), i_(b.i_)
    {
    }

    bool operator==(const iterator &b)
    {
      return i_ == b.i_;
    }
    bool operator!=(const iterator &b)
    {
      return !operator==(b);
    }

    const Key &operator*() const
    {
      return set_->idx_to_val_[i_];
    }

    iterator &operator++()
    {
      i_++;

      while (i_ < set_->idx_to_val_.size() && !set_->freemap_[i_]) {
        i_++;
      }

      return *this;
    }

  private:
    const OrderedSet *set_;
    int i_;
  };

  OrderedSet()
  {
  }

  bool add(const Key &key)
  {
    if (!val_to_idx_.contains(key)) {
      if (freelist_.size()) {
        int i = freelist_.pop_back();

        val_to_idx_.add(key, i);
        freemap_.set(i, false);
        idx_to_val_[i] = key;
      } else {
        val_to_idx_.add(key, idx_to_val_.size());
        idx_to_val_.append(key);
      }

      size_++;
    }

    return false;
  }

  bool remove(const Key &key)
  {
    int i = 0;

    if (val_to_idx_.remove(key, &i)) {
      freelist_.append(i);
      freemap_.set(i, true);
      size_--;
    }
  }

  size_t size() const
  {
    return size_;
  }

  iterator begin() const
  {
    return iterator(this, 0);
  }

  iterator end() const
  {
    return iterator(this, int(idx_to_val_.size()));
  }

private:
  Map<Key, int, static_size> val_to_idx_;
  Vector<Key, static_size> idx_to_val_;

  size_t size_ = 0;
  Vector<int, static_size> freelist_;
  BoolVector<static_size * 32> freemap_;
};
} // namespace litestl::util
