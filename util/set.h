#pragma once

#include "boolvector.h"
#include "compiler_util.h"
#include "hash.h"
#include "map.h"
#include "vector.h"

#include "hashtable_sizes.h"
#include <cstdint>
#include <span>

namespace litestl::util {

template <typename Key, size_t static_size_logical = 4> struct Set {
  using key_type = Key;
  static constexpr size_t static_size = static_size_logical * 4;

  struct iterator {
    using key_type = Key;

    iterator(const Set *set, int i) : set_(set), i_(i)
    {
      if (i == 0) {
        /* Prewind iterator to first key. */
        i_--;
        operator++();
      }
    }
    iterator(const iterator &b) : set_(b.set_), i_(b.i_)
    {
    }

    bool operator==(const iterator &b) const
    {
      return b.i_ == i_;
    }
    bool operator!=(const iterator &b) const
    {
      return !operator==(b);
    }

    const Key &operator*() const
    {
      return set_->table_[i_];
    }

    iterator &operator++()
    {
      i_++;

      while (i_ < set_->table_.size() && !set_->usedmap_[i_]) {
        i_++;
      }

      return *this;
    }

  private:
    const Set *set_;
    int i_;
  };

  Set()
  {
    realloc(hashsizes[find_hashsize_prev(static_size)]);
  }

  ~Set()
  {
    if constexpr (!is_simple<Key>()) {
      for (int i = 0; i < table_.size(); i++) {
        if (usedmap_[i]) {
          table_[i].~Key();
        }
      }
    }

    if (static_cast<void *>(table_.data()) != static_cast<void *>(static_storage_)) {
      alloc::release(static_cast<void *>(table_.data()));
    }
  }

  iterator begin()
  {
    return iterator(this, 0);
  }
  iterator end()
  {
    return iterator(this, table_.size());
  }

  bool add(const Key &key)
  {
    check_capacity();

    int i = find_cell(key);

    if (!usedmap_[i]) {
      usedmap_.set(i, true);
      table_[i] = key;

      size_++;

      return true;
    }

    return false;
  }

  bool remove(const Key &key)
  {
    int i = find_cell(key);

    if (usedmap_[i]) {
      table_[i].~Key();
      size_--;
      usedmap_.set(i, false);

      return true;
    }

    return false;
  }

  bool contains(const Key &key) const
  {
    int i = find_cell(key);
    return usedmap_[i];
  }

  bool operator[](const Key &key) const
  {
    return contains(key);
  }

  size_t size()
  {
    return size_;
  }

private:
  void check_capacity()
  {
    if (size_ + 1 >= max_size_) {
      realloc((max_size_ + 1) * 9);
    }
  }

  int find_cell(const Key &key) const
  {
    const hash::HashInt size = hash::HashInt(table_.size());
    hash::HashInt hashval = hash::hash(key) % size;
    hash::HashInt h = hashval, probe = hashval;

    while (1) {
      if (!usedmap_[h] || table_[h] == key) {
        return h;
      }

      probe++;
      hashval += probe;
      h = hashval % size;
    }
  }

  void realloc(size_t size)
  {
    cursize_ = find_hashsize(size);
    size = hashsizes[cursize_];

    std::span<Key> old = table_;
    BoolVector<> usedmap_old = usedmap_;

    if (size < static_size) {
      table_ = {reinterpret_cast<Key *>(static_storage_), size};
    } else {
      table_ = {static_cast<Key *>(alloc::alloc("Set table", sizeof(Key) * size)), size};
    }

    max_size_ = size / 3;
    usedmap_.resize(size);
    usedmap_.clear();

    int oldsize = old.size();
    for (int i = 0; i < oldsize; i++) {
      if (!(usedmap_old[i])) {
        continue;
      }

      int new_i = find_cell(old[i]);
      table_[new_i] = std::move(old[i]);
      usedmap_.set(new_i, true);

      if constexpr (!is_simple<Key>()) {
        old[i].~Key();
      }
    }

    if (old.data() && old.data() != reinterpret_cast<Key *>(static_storage_)) {
      alloc::release(static_cast<void *>(old.data()));
    }
  }

  std::span<Key> table_;
  int cursize_ = 0;                /* index into hashsizes[] */
  size_t size_ = 0, max_size_ = 0; /* hashsizes[cursize_]*3 */
  char static_storage_[sizeof(Key) * static_size];
  BoolVector<> usedmap_;
};
} // namespace litestl::util
