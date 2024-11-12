#pragma once

#include "alloc.h"
#include "boolvector.h"
#include "compiler_util.h"
#include "concepts.h"
#include "hash.h"
#include "hashtable_sizes.h"

#include <concepts>
#include <initializer_list>
#include <span>
#include <type_traits>

namespace litestl::util {

namespace detail::map {

template <typename Func, typename Key>
/* clang-format off */
concept KeyCopier = requires(Func f, Key k)
{
  {f(k)} -> IsAnyOf<Key, Key&, Key&&, const Key&>;
};
/* clang-format on */
} // namespace detail::map

namespace detail::map {
  template <typename Key, typename Value> struct Pair {
  Key key;
  Value value;

  Pair()
  {
  }

  Pair(Pair &&b) = default;
  Pair(const Pair &b) = default;

  Pair(Key &key_, Value &value_)
  {
    key = key_;
    value = value_;
  }
  Pair(Key &&key_, Value &&value_)
  {
    key = key_;
    value = value_;
  }

  Pair &operator=(Pair &&b)
  {
    if (this == &b) {
      return *this;
    }

    key = std::move(key);
    value = std::move(value);

    return *this;
  }

  Pair &operator=(const Pair &b)
  {
    if (this == &b) {
      return *this;
    }

    key = b.key;
    value = b.value;

    return *this;
  }

  static constexpr bool is_simple()
  {
    return (std::is_integral_v<Key> || std::is_pointer_v<Key>) &&
           (std::is_integral_v<Value> || std::is_floating_point_v<Value> ||
            std::is_pointer_v<Value>);
  }
};
} // namespace detail
template <typename Key, typename Value, int static_size = 16> class alignas(ContainerAlign<detail::map::Pair<Key, Value>>()) Map {
  using Pair = detail::map::Pair<Key, Value>;
  const static int real_static_size = static_size * 3 + 1;

public:
  using key_type = Key;
  using value_type = Value;

  struct iterator {
    iterator(const Map *map, int i) : map_(map), i_(i)
    {
      if (i_ == 0) {
        /* Find first item. */
        i_ = -1;
        operator++();
      }
    }

    iterator(const iterator &b) : map_(b.map_), i_(b.i_)
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

    const Pair &operator*() const
    {
      return map_->table_[i_];
    }

    iterator &operator++()
    {
      i_++;

      while (i_ < map_->table_.size() && !map_->used_[i_]) {
        i_++;
      }

      return *this;
    }

  private:
    int i_;
    const Map *map_;
  };

  template <bool is_key, typename T> struct key_value_range {
    key_value_range(const Map *map, int i = 0) : map_(map), i_(i)
    {
      if (i == 0) {
        i_--;
        operator++();
      }
    }

    key_value_range(const key_value_range &b) : map_(b.map_), i_(b.i_)
    {
    }

    bool operator==(const key_value_range &b)
    {
      return b.i_ == i_;
    }

    bool operator!=(const key_value_range &b)
    {
      return !operator==(b);
    }

    T &operator*()
    {
      if constexpr (is_key) {
        return map_->table_[i_].key;
      } else {
        return map_->table_[i_].value;
      }
    }

    key_value_range &operator++()
    {
      i_++;

      int table_size = int(map_->table_.size());

      while (i_ < map_->table_.size() && !map_->used_[i_]) {
        i_++;
      }

      return *this;
    }

    key_value_range begin() const
    {
      return key_value_range(map_, 0);
    }

    key_value_range end() const
    {
      return key_value_range(map_, map_->table_.size());
    }

  private:
    const Map *map_;
    int i_ = 0;
  };

  using key_range = key_value_range<true, Key>;
  using value_range = key_value_range<false, Value>;

  friend struct key_value_range<true, Key>;

  Map(const Map &b) : cur_size_(b.cur_size_)
  {
    int size = hashsizes[cur_size_];

    if (size <= real_static_size) {
      table_ = std::span(get_static(), size);
    } else {
      table_ = std::span(
          static_cast<Pair *>(alloc::alloc("copied map table", sizeof(Pair) * size)),
          size);
    }

    used_ = b.used_;
    used_count_ = b.used_count_;

    for (int i = 0; i < size; i++) {
      if (used_[i]) {
        table_[i] = b.table_[i];
      }
    }
  }

  Map()
      : table_(get_static(), hashsizes[find_hashsize_prev(real_static_size)]),
        cur_size_(find_hashsize_prev(real_static_size))
  {
    reserve_usedmap();
  }

  Map(Map &&b)
  {
    if (table_.data() == get_static()) {
      Map(static_cast<const Map &>(b));
    } else {
      table_ = std::move(b.table_);
      used_count_ = b.used_count_;
      used_ = std::move(b.used_);
      cur_size_ = b.cur_size_;
    }
  }

  DEFAULT_MOVE_ASSIGNMENT(Map)

  Map(std::initializer_list<Pair> list)
  {
    cur_size_ = find_hashsize(list.size() * 3 + 1);
    int size = hashsizes[cur_size_];

    if (size <= real_static_size) {
      table_ = std::span(get_static(), size);
    } else {
      table_ = std::span(
          static_cast<Pair *>(alloc::alloc("Map table", sizeof(Pair) * size)), size);
    }

    reserve_usedmap();

    for (auto &&item : list) {
      add_overwrite(item.key, item.value);
    }
  }

  ~Map()
  {
    if (table_.data() == get_static()) {
      return;
    }

    if constexpr (!Pair::is_simple()) {
      for (int i = 0; i < table_.size(); i++) {
        if (used_[i]) {
          table_[i].~Pair();
        }
      }
    }

    alloc::release(static_cast<void *>(table_.data()));
  }

  key_range keys() const
  {
    return key_range(this);
  }

  value_range values() const
  {
    return value_range(this);
  }

  iterator begin() const
  {
    return iterator(this, 0);
  }

  iterator end() const
  {
    return iterator(this, table_.size());
  }

  size_t size() const
  {
    return size_t(used_count_);
  }

  /* Does not check if key already exists. */
  void insert(const Key &key, const Value &value)
  {
    check_load();

    int i = find_pair<false, true>(key);
    add_finalize(i, key, value);
  }

  /* Does not check if key already exists. */
  void insert(Key &&key, Value &&value)
  {
    check_load();

    int i = find_pair<false, true>(key);
    add_finalize(i, key, value);
  }

  bool add(const Key &key, const Value &value)
  {
    return add_intern<false>(key, value);
  }
  bool add(const Key &&key, const Value &&value)
  {
    return add_intern<false>(key, value);
  }

  bool add_overwrite(const Key &key, const Value &value)
  {
    return add_intern<true>(key, value);
  }
  bool add_overwrite(const Key &&key, const Value &&value)
  {
    return add_intern<true>(key, value);
  }

  Value &operator[](const Key &key)
  {
    check_load();
    int i = find_pair<true, true>(key);

    if (!used_[i]) {
      if constexpr (!is_simple<Value>()) {
        new (static_cast<void *>(&table_[i].value)) Value();
      }

      used_.set(i, true);
      new (static_cast<void *>(&table_[i].key)) Key(key);
      used_count_++;
    }
    return table_[i].value;
  }

  bool contains(const Key &key) const
  {
    int i = find_pair<true, false>(key);
    return i != -1;
  }
  bool contains(const Key &&key) const
  {
    int i = find_pair<true, false>(key);
    return i != -1;
  }

  Value *lookup_ptr(const Key &key) const
  {
    int i = find_pair<true, false>(key);
    if (i < 0) {
      return nullptr;
    }

    return &table_[i].value;
  }

  template <detail::map::KeyCopier<Key> KeyCopyFunc, typename ValueSetFunc>
  Value &add_callback(const Key &key, KeyCopyFunc copy_key, ValueSetFunc set_value)
  {
    check_load();

    int i = find_pair<true, true>(key);
    if (!used_[i]) {
      used_.set(i, true);
      used_count_++;

      /* Use copy/move constructors since we have unallocated memory. */
      new (static_cast<void *>(&table_[i].key)) Key(copy_key(key));
      new (static_cast<void *>(&table_[i].value)) Value(set_value());
    }

    return table_[i].value;
  }

  bool add_uninitialized(const Key &key, Value **value)
  {
    check_load();

    int i = find_pair<true, true>(key);

    if (value) {
      new (static_cast<void *>(&table_[i].value)) Value();
      *value = &table_[i].value;
    }

    if (!used_[i]) {
      // use placement new instead of assignment
      new (static_cast<void *>(&table_[i].key)) Key(key);
      used_.set(i, true);
      used_count_++;
      return true;
    }

    return false;
  }

  /* Undefined behavior if key is not in map, check existence with .contains(). */
  Value &lookup(const Key &key) const
  {
    int i = find_pair<true, false>(key);
    return table_[i].value;
  }

  bool remove(const Key &key, Value *out_value = nullptr)
  {
    int i = find_pair<true, false>(key);

    if (i == -1) {
      return false;
    }

    this->used_.set(i, false);

    if (out_value) {
      *out_value = std::move(table_[i].value);
    }

    if constexpr (!Pair::is_simple()) {
      table_[i].~Pair();
    }

    return true;
  }

  void reserve(size_t size)
  {
    size = size * 3 + 1;

    if (table_.size() >= size) {
      return;
    }

    realloc_to_size(size);
  }

private:
  using MyBoolVector = BoolVector<static_size * 3 + 1>;

  std::span<Pair> table_;
  char *static_storage_[real_static_size * sizeof(Pair)];
  MyBoolVector used_;
  int cur_size_ = 0;
  int used_count_ = 0;

  void reserve_usedmap()
  {
    hash::HashInt size = hash::HashInt(hashsizes[cur_size_]);
    used_.resize(size);
    used_.clear();
  }

  template <bool overwrite = false> bool add_intern(const Key &key, const Value &value)
  {
    check_load();

    int i = find_pair<true, true>(key);
    if (used_[i]) {
      if constexpr (overwrite) {
        add_finalize(i, key, value);
      }

      return false;
    }

    add_finalize(i, key, value);

    return true;
  }

  /* Handles rvalues. */
  template <typename KeyArg, typename ValueArg>
  void add_finalize(int i, const KeyArg key, const ValueArg value)
  {
    if constexpr (!Pair::is_simple()) {
      if (!used_[i]) {
        /* Use copy/move constructors. */
        new (static_cast<void *>(&table_[i].key)) Key(key);
        new (static_cast<void *>(&table_[i].value)) Value(value);
      } else {
        table_[i].key = key;
        table_[i].value = value;
      }
    } else {
      table_[i].key = key;
      table_[i].value = value;
    }

    used_count_++;
    used_.set(i, true);
  }

  template <bool check_key_equals = true, bool return_unused_cell = false>
  ATTR_NO_OPT int find_pair(const Key &key) const
  {
    const hash::HashInt size = hash::HashInt(table_.size());
    hash::HashInt hashval = hash::hash(key) % size;
    hash::HashInt h = hashval, probe = hashval;

    while (1) {
      if (!used_[h]) {
        if constexpr (return_unused_cell) {
          return h;
        } else {
          return -1;
        }
      }

      if constexpr (check_key_equals) {
        if (table_[h].key == key) {
          return h;
        }
      }

      probe++;
      hashval += probe;
      h = hashval % size;
    }
  }

  inline ATTR_NO_OPT bool check_load()
  {
    if (used_count_ > table_.size() / 3) {
      realloc_to_size(table_.size() * 3);
      return true;
    }

    return false;
  }

  inline ATTR_NO_OPT void realloc_to_size(size_t size)
  {
    size_t old_size = hashsizes[cur_size_];
    while (hashsizes[cur_size_] < size) {
      cur_size_++;
    }

    size_t newsize = hashsizes[cur_size_];

    std::span<Pair> old = table_;
    MyBoolVector old_used = used_;

    table_ = std::span(static_cast<Pair *>(alloc::alloc("sculpecore::util::map table",
                                                        newsize * sizeof(Pair))),
                       newsize);

    used_count_ = 0;
    used_.resize(newsize);
    used_.clear();

    for (int i = 0; i < old.size(); i++) {
      if (old_used[i]) {
        int index = find_pair<false, true>(old[i].key);

        new (static_cast<void *>(&table_[index].key)) Key(std::move(old[i].key));
        new (static_cast<void *>(&table_[index].value)) Value(std::move(old[i].value));

        if constexpr (!Pair::is_simple()) {
          old[i].~Pair();
        }
        used_.set(index, true);
      }
    }

    if (old.data() != get_static()) {
      alloc::release(static_cast<void *>(old.data()));
    }
  }

  Pair *get_static()
  {
    return reinterpret_cast<Pair *>(static_storage_);
  }
};
} // namespace litestl::util
