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

// cannot rely on pointer members forcibly aligning to 8
// because of wasm
template <typename Key, size_t static_size_logical = 4>
struct alignas(ContainerAlign<Key>()) Set {
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

  Set(Set &&b)
      : usedmap_(std::move(b.usedmap_)), cursize_(b.cursize_), size_(b.size_),
        clearmap_(std::move(b.clearmap_)), max_size_(b.max_size_)
  {
    if (static_cast<const void *>(b.table_.data()) ==
        static_cast<const void *>(b.static_storage_))
    {
      table_ = {reinterpret_cast<Key *>(static_storage_), b.table_.size()};

      if constexpr (!is_simple<Key>()) {
        for (int i = 0; i < b.table_.size(); i++) {
          if (usedmap_[i]) {
            new (static_cast<void *>(&table_[i])) Key(std::move(b.table_[i]));
          }
        }
      } else {
        memcpy(static_storage_, b.static_storage_, sizeof(static_storage_));
      }
    } else {
      table_ = b.table_;
    }

    b.table_ = nullptr;
    b.size_ = 0;
  }

  Set(const Set &b)
      : usedmap_(b.usedmap_), clearmap_(b.clearmap_), cursize_(b.cursize_),
        size_(b.size_), max_size_(b.max_size_)
  {
    if (static_cast<const void *>(b.table_.data()) ==
        static_cast<const void *>(b.static_storage_))
    {
      table_ = {reinterpret_cast<Key *>(static_storage_), b.table_.size()};
    } else {
      table_ = {
          static_cast<Key *>(alloc::alloc("Set table", sizeof(Key) * b.table_.size())),
          b.table_.size()};
    }

    if constexpr (!is_simple<Key>()) {
      for (int i = 0; i < b.table_.size(); i++) {
        if (usedmap_[i]) {
          new (static_cast<void *>(&table_[i])) Key(b.table_[i]);
        }
      }
    } else {
      memcpy(static_cast<void *>(table_.data()),
             static_cast<void *>(b.table_.data()),
             sizeof(Key) * b.table_.size());
    }
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

  DEFAULT_MOVE_ASSIGNMENT(Set)
  DEFAULT_COPY_ASSIGNMENT(Set)

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

    int first_clearcell = -1;
    int i = find_cell(key, first_clearcell);

    if (!usedmap_[i]) {
      // if we hit a clear cell in the chain, use it
      // and clear it's clearcell bit.
      if (first_clearcell != -1) {
        i = first_clearcell;
        clearmap_.set(first_clearcell, false);
      }

      usedmap_.set(i, true);
      table_[i] = key;

      size_++;

      return true;
    }

    return false;
  }

  bool remove(const Key &key)
  {
    int first_clearcell = -1;
    int i = find_cell(key, first_clearcell);

    if (usedmap_[i]) {
      table_[i].~Key();
      size_--;
      usedmap_.set(i, false);
      clearmap_.set(i, true);

      return true;
    }

    return false;
  }

  bool contains(const Key &key) const
  {
    int first_clearcell = -1;
    int i = find_cell(key, first_clearcell);
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

  Set &clear()
  {
    if constexpr (!is_simple<Key>()) {
      int size = table_.size();
      for (int i = 0; i < size; i++) {
        if (!(usedmap_[i])) {
          continue;
        }

        table_[i].~Key();
      }
    }

    size_ = 0;
    usedmap_.clear();
    return *this;
  }

private:
  void check_capacity()
  {
    if (size_ + 1 >= max_size_) {
      realloc((max_size_ + 1) * 9);
    }
  }

  int find_cell(const Key &key, int &first_clearcell) const
  {
    return const_cast<Set *>(this)->find_cell<false>(key, first_clearcell);
  }

  template <bool realloc_clearcells = true>
  int find_cell(const Key &key, int &first_clearcell)
  {
    const hash::HashInt size = hash::HashInt(table_.size());
    hash::HashInt hashval = hash::hash(key) % size;
    hash::HashInt h = hashval, probe = hashval;

    int count = 0;
    while (1) {
      if (count > size) {
        // clearcell chain is full
        if constexpr (realloc_clearcells) {
          clear_clearcell();
        }
        return find_cell<realloc_clearcells>(key, first_clearcell);
      }
      // hit free cell?
      if (!usedmap_[h]) {
        // was this cell previously cleared? if so keep looking
        if (clearmap_[h]) {
          first_clearcell = h;
        } else {
          return h;
        }
      } else if (table_[h] == key) {
        return h;
      }

      probe++;
      hashval += probe;
      h = hashval % size;
      count++;
    }
  }

  void clear_clearcell()
  {
    realloc(table_.size());
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
    clearmap_.resize(size);
    clearmap_.clear();

    int oldsize = old.size();
    for (int i = 0; i < oldsize; i++) {
      if (!(usedmap_old[i])) {
        continue;
      }

      int first_clearcell = -1;
      int new_i = find_cell(old[i], first_clearcell);
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
  BoolVector<> clearmap_;
};
} // namespace litestl::util
