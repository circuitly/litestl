#pragma once
#include "alloc.h"
#include "compiler_util.h"
#include "concepts.h"
#include "index_range.h"
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>

template <typename Iter, typename QualVec, typename QualT>
static Iter operator+(int n, const Iter &b)
{
  return Iter(b.vector(), n + b.index());
}
namespace litestl::util {

template <typename BASE, typename ITEM>
concept VectorSortComparator = requires(BASE base, const ITEM &a, const ITEM &b) {
  {
    base(a, b)
  } -> std::convertible_to<std::int32_t>;
};

namespace detail {
template <typename T, VectorSortComparator<T> CB> struct Comparator {
  CB &cb;

  Comparator(CB &cb_) : cb(cb_)
  {
    //
  }

  bool operator()(const T &a, const T &b)
  {
    return cb(a, b) < 0;
  }
};
} // namespace detail

static constexpr int VectorDefaultStaticSize = 1;
template <typename T, int static_size = VectorDefaultStaticSize>
/** asdsa */
class alignas(ContainerAlign<T>()) Vector {
public:
  using value_type = T;

  template <typename Iter> struct iterator_diff {
    flatten_inline iterator_diff()
    {
    }
    flatten_inline iterator_diff(int i) : i_(i)
    {
    }
    flatten_inline iterator_diff(const iterator_diff &b) : i_(b.i_)
    {
    }

    operator int()
    {
      return i_;
    }

    flatten_inline iterator_diff &operator=(iterator_diff &b)
    {
      i_ = b.i_;
      return *this;
    }
    flatten_inline iterator_diff &operator=(int b)
    {
      i_ = b;
      return *this;
    }

    flatten_inline Iter operator+(const Iter &b) const
    {
      return Iter(b.vector(), i_ + b.index());
    }
    flatten_inline iterator_diff operator+(const iterator_diff &b) const
    {
      return iterator_diff(i_ + b.i_);
    }
    flatten_inline iterator_diff operator+(int b) const
    {
      return iterator_diff(i_ + b);
    }
    flatten_inline iterator_diff &operator+=(const Iter &b)
    {
      i_ += b.index();
      return *this;
    }
    flatten_inline iterator_diff &operator+=(const iterator_diff &b)
    {
      i_ += b.i_;
      return *this;
    }
    flatten_inline iterator_diff &operator+=(int b)
    {
      i_ += b;
      return *this;
    }

    flatten_inline Iter operator-(const Iter &b) const
    {
      return Iter(b.vector(), i_ - b.index());
    }
    flatten_inline iterator_diff operator-(const iterator_diff &b) const
    {
      return iterator_diff(i_ - b.i_);
    }
    flatten_inline iterator_diff operator-(int b) const
    {
      return iterator_diff(i_ - b);
    }
    flatten_inline iterator_diff &operator-=(const Iter &b)
    {
      i_ -= b.index();
      return *this;
    }
    flatten_inline iterator_diff &operator-=(const iterator_diff &b)
    {
      i_ -= b.i_;
      return *this;
    }
    flatten_inline iterator_diff &operator-=(int b)
    {
      i_ -= b;
      return *this;
    }

    flatten_inline auto operator<=>(const iterator_diff &b) const
    {
      return i_ <=> b.i_;
    }

    flatten_inline int value() const
    {
      return i_;
    }

  private:
    int i_;
  };
  template <typename QualifiedVector, typename QualT> struct iterator_base {
    using difference_type = int;
    using value_type = QualT;

    flatten_inline QualifiedVector &vector()
    {
      return *vec_;
    }
    flatten_inline int index()
    {
      return i_;
    }

    // default constructor deliberately creates uninitialized memory
    flatten_inline iterator_base()
    {
    }

    flatten_inline iterator_base(QualifiedVector &vec, int i) : i_(i), vec_(&vec)
    {
    }

    flatten_inline iterator_base(const iterator_base &b) : i_(b.i_), vec_(b.vec_)
    {
    }

    flatten_inline iterator_base(iterator_base &&b) : i_(b.i_), vec_(b.vec_)
    {
      b.vec_ = nullptr;
    }

    iterator_base &operator=(const iterator_base &b)
    {
      i_ = b.i_;
      vec_ = b.vec_;
      return *this;
    }
    iterator_base &operator=(iterator_base &&b)
    {
      i_ = b.i_;
      vec_ = b.vec_;
      b.vec_ = nullptr;
      return *this;
    }

    flatten_inline bool operator==(const iterator_base &b) const
    {
      return b.i_ == i_;
    }
    flatten_inline bool operator!=(const iterator_base &b) const
    {
      return b.i_ != i_;
    }

    flatten_inline QualT &operator*() const
    {
      return vec_->data_[i_];
    }
    // preincrement
    flatten_inline iterator_base &operator++()
    {
      iterator_base a = 5 - *this;
      i_++;
      return *this;
    }
    // postincrement
    flatten_inline iterator_base operator++(int arg)
    {
      i_++;
      return iterator_base(vec_, i_ - 1);
    }
    // preincrement
    flatten_inline iterator_base &operator--()
    {
      i_--;
      return *this;
    }
    // postincrement
    flatten_inline iterator_base operator--(int arg)
    {
      i_--;
      return iterator_base(vec_, i_ + 1);
    }

    flatten_inline auto operator<=>(const iterator_base &b) const
    {
      return i_ <=> b.i_;
    }

    flatten_inline iterator_base &operator+=(difference_type b)
    {
      i_ += b;
      return *this;
    }
    flatten_inline iterator_base &operator-=(difference_type b)
    {
      i_ -= b;
      return *this;
    }

    flatten_inline QualT &operator[](int i) const
    {
      return vec_->data_[i];
    }

    friend difference_type operator-(const iterator_base &a, const iterator_base &b)
    {
      return a.i_ - b.i_;
    }
    friend iterator_base operator+(int n, const iterator_base &b)
    {
      return iterator_base(*b.vec_, n + b.i_);
    }
    friend iterator_base operator+(const iterator_base &b, int n)
    {
      return iterator_base(*b.vec_, b.i_ + n);
    }
    friend iterator_base operator-(int n, const iterator_base &b)
    {
      return iterator_base(*b.vec_, n - b.i_);
    }
    friend iterator_base operator-(const iterator_base &b, int n)
    {
      return iterator_base(*b.vec_, b.i_ - n);
    }

  private:
    int i_;
    QualifiedVector *vec_;
  };

  using iterator = iterator_base<Vector, T>;
  using const_iterator = iterator_base<const Vector, const T>;
  using span = std::span<T>;
  using const_span = const std::span<const T>;

  flatten_inline Vector(std::initializer_list<T> list)
  {
    if (list.size() > static_size) {
      data_ = static_cast<T *>(alloc::alloc("Vector data", sizeof(T) * list.size()));
      capacity_ = size_ = list.size();
    } else {
      data_ = reinterpret_cast<T *>(static_storage_);
      capacity_ = size_ = static_size;
    }

    size_ = list.size();
    int i = 0;
    for (auto &&item : list) {
      new (static_cast<void *>(data_ + i)) T(std::move(item));
      i++;
    }
  }

  /** Initialize from an existing array of items.
   *  Items in `items` will be moved into newly
   *  allocated memory (or the local static
   *  storage if `itemCount < static_size`).
   */
  Vector(T *items, int itemCount)
  {
    if (itemCount <= static_size) {
      capacity_ = static_size;
      size_ = 0;
      data_ = reinterpret_cast<T *>(static_storage_);
      for (int i = 0; i < itemCount; i++) {
        this->append(std::move(items[i]));
      }
    } else {
      data_ = static_cast<T *>(alloc::alloc("Vector data", sizeof(T) * itemCount));
      capacity_ = size_ = itemCount;

      if constexpr (!is_simple<T>()) {
        for (int i = 0; i < itemCount; i++) {
          new (static_cast<void *>(data_ + i)) T(std::move(items[i]));
        }
      } else {
        memcpy(static_cast<void *>(data_),
               static_cast<void *>(items),
               sizeof(T) * itemCount);
      }
    }
  }

  Vector() : capacity_(static_size)
  {
    data_ = static_storage();
  }

  operator span() {
    return std::span<T>(data_, size_);
  }
  
  operator span() const {
    return std::span<const T>(data_, size_);
  }

  ~Vector()
  {
    if (data_ != nullptr) {
      release_data(data_, size_);
    }
  }

  Vector(const Vector &b)
  {
    size_ = b.size_;
    capacity_ = b.capacity_;

    if (size_ > static_size) {
      data_ = static_cast<T *>(alloc::alloc("Vector", sizeof(T) * b.capacity_));
    } else {
      data_ = static_storage();
    }

    for (int i = 0; i < size_; i++) {
      // use copy constructor
      new (static_cast<void *>(data_ + i)) T(std::forward<T &>(b.data_[i]));
    }
  }

  void clear()
  {
    resize<true, true>(0);
  }

  void clear_and_contract()
  {
    deconstruct_all();
    size_ = 0;
    contract();
  }

  template <VectorSortComparator<T> CB> void sort(CB cb)
  {
    std::ranges::sort(
        iterator(*this, 0), iterator(*this, size_), detail::Comparator<T, CB>(cb));
  }
  void sort()
  {
    std::ranges::sort(iterator(*this, 0), iterator(*this, size_), std::ranges::less());
  }

  bool hasStaticStorage() const
  {
    return data_ == reinterpret_cast<const T *>(static_storage_);
  }

  /** Reallocates vector to have no spare capacity. */
  void contract()
  {
    if (size_ == capacity_) {
      return;
    }

    if (size_ == 0) {
      if (data_ && data_ != static_storage()) {
        alloc::release(static_cast<void *>(data_));
      }

      capacity_ = static_size;
      data_ = static_storage();
    }

    T *newdata;

    if (size_ < static_size) {
      capacity_ = static_size;

      if (data_ == static_storage()) {
        /* We're already inside the static storage, do nothing. */
        return;
      }

      newdata = static_storage();
    } else {
      capacity_ = size_;
      newdata = static_cast<T *>(alloc::alloc("Vector", sizeof(T) * size_));
    }

    if (!is_simple<T>()) {
      for (int i = 0; i < size_; i++) {
        newdata[i] = std::move(data_[i]);
      }

      deconstruct_all();
    } else {
      memcpy(static_cast<void *>(newdata), static_cast<void *>(data_), sizeof(T) * size_);
    }

    if (data_ != static_storage()) {
      alloc::release(static_cast<void *>(data_));
    }

    data_ = newdata;
  }

  Vector &operator=(const Vector &b)
  {
    if (&b == this) {
      return *this;
    }

    deconstruct_all();

    resize<false>(b.size());

    for (int i = 0; i < size_; i++) {
      data_[i] = b.data_[i];
    }

    return *this;
  }

  DEFAULT_MOVE_ASSIGNMENT(Vector)

  Vector(Vector &&b)
  {
    size_ = b.size_;
    capacity_ = b.capacity_;

    if (size_ <= static_size) {
      data_ = static_storage();

      for (int i = 0; i < size_; i++) {
        if constexpr (!is_simple<T>()) {
          new (static_cast<void *>(data_ + i)) T(std::move(b.data_[i]));
        } else {
          data_[i] = std::move(b.data_[i]);
        }
      }

      if (b.data_ && b.data_ != b.static_storage()) {
        alloc::release(static_cast<void *>(b.data_));
      }
    } else {
      data_ = b.data_;
    }

    b.capacity_ = 0;
    b.data_ = nullptr;
    b.size_ = 0;
  }

public:
  const_iterator begin() const
  {
    return const_iterator(*this, 0);
  }
  const_iterator end() const
  {
    return const_iterator(*this, size_);
  }

  iterator begin()
  {
    return iterator(*this, 0);
  }
  iterator end()
  {
    return iterator(*this, size_);
  }

  bool contains(const T &value) const
  {
    return index_of(value) != -1;
  }

  T pop_back()
  {
    size_--;

    T ret = std::move(data_[size_]);
    if constexpr (!is_simple<T>()) {
      data_[size_].~T();
    }

    return ret;
  }

  T pop_front()
  {
    T ret = std::move(data_[0]);
    for (int i = 0; i < size_ - 1; i++) {
      data_[i] = std::move(data_[i + 1]);
    }
    size_--;
    return ret;
  }

  bool remove(const T &value, bool swap_end_only = false)
  {
    return remove_intern<const T &>(value, swap_end_only);
  }

  template <typename TArg> bool remove_intern(TArg value, bool swap_end_only = false)
  {
    int i = index_of(value);
    if (i < 0) {
      fprintf(stderr, "Item not in list\n");
      return false;
    }
    return remove_at(i, swap_end_only);
  }

  bool remove_at(int i, bool swap_end_only = false)
  {
    if constexpr (!is_simple<T>()) {
      data_[i].~T();
    }

    if (swap_end_only) {
      data_[i] = std::move(data_[size_ - 1]);
    } else {
      while (i < size_ - 1) {
        data_[i] = std::move(data_[i + 1]);
        i++;
      }

      /* Run end's destructor even though we moved its contents. */
      if constexpr (!is_simple<T>()) {
        data_[i].~T();
      }
    }

    size_--;
    return true;
  }

  int index_of(const T &value) const
  {
    for (int i = 0; i < size_; i++) {
      if (data_[i] == value) {
        return i;
      }
    }

    return -1;
  }

  bool append_once(T &&value)
  {
    if (index_of(value) == -1) {
      append(value);
      return true;
    }

    return false;
  }

  bool append_once(const T &value)
  {
    if (index_of(value) == -1) {
      append(value);
      return true;
    }

    return false;
  }

  template <typename... Args> T &grow_one(Args... args)
  {
    T &result = append_intern();
    if constexpr (!is_simple<T>()) {
      new (static_cast<void *>(&result)) T(std::forward<Args>(args)...);
    }
    return result;
  }

  void append(const T &value)
  {
    new (static_cast<void *>(&append_intern())) T(value);
  }

  void append(T &&value)
  {
    new (static_cast<void *>(&append_intern())) T(std::forward<T &&>(value));
  }

  void ensure_capacity(size_t size)
  {
    ensure_size(size);
  }

  template <bool construct_destruct = true, bool shrink_only = false> void resize(size_t newsize)
  {
    size_t remain = 0;
    if (newsize > size_) {
      remain = newsize - size_;
    } else if (newsize < size_) {
      if constexpr (construct_destruct && !is_simple<T>()) {
        for (int i = newsize; i < size_; i++) {
          data_[i].~T();
        }
      }
    }

    ensure_size(newsize);
    size_ = newsize;

    /* Construct new elements. */
    if constexpr (construct_destruct && !shrink_only) {
      for (int i = 0; i < remain; i++) {
        if constexpr (!is_simple<T>()) {
          new (&data_[size_ - i - 1]) T;
        } else {
          data_[size_ - i - 1] = T(0);
        }
      }
    }
  }

  T &operator[](int idx)
  {
    return data_[idx];
  }

  const T &operator[](int idx) const
  {
    return data_[idx];
  }

  size_t size() const
  {
    return size_;
  }

  T &last()
  {
    return data_[size_ - 1];
  }

  T *data()
  {
    return data_;
  }

  /**
   *  cs the vector in-place.
   *  Returns a reference to *this.
   */
  Vector<T, static_size> &reverse()
  {
    int size = size_ >> 1;
    for (int i = 0; i < size; i++) {
      std::swap(data_[i], data_[size_ - i - 1]);
    }
    return *this;
  }

private:
  flatten_inline void deconstruct_all()
  {
    if constexpr (!is_simple<T>()) {
      for (int i = 0; i < size_; i++) {
        data_[i].~T();
      }
    }
  }

  ATTR_NO_OPT T &append_intern()
  {
    ensure_size(size_ + 1);
    size_++;
    if (size_ < 0 || reinterpret_cast<intptr_t>(data_) < 100) {
      printf("error!\n");
    }
    return data_[size_ - 1];
  }

  ATTR_NO_OPT void ensure_size(size_t newsize)
  {
    if (newsize < capacity_) {
      return;
    }

    size_t new_capacity = (newsize + 1) << 1;
    new_capacity -= newsize >> 1;
    capacity_ = new_capacity;
    T *old = data_;

    data_ = static_cast<T *>(
        static_cast<void *>(alloc::alloc("Vector data", new_capacity * sizeof(T))));

    if constexpr (is_simple<T>()) {
      memcpy(static_cast<void *>(data_), static_cast<void *>(old), sizeof(T) * size_);
    } else {
      for (int i = 0; i < size_; i++) {
        new (static_cast<void *>(data_ + i)) T(std::move(old[i]));
      }
    }

    release_data(old, size_);
  }

  ATTR_NO_OPT void release_data(T *old, int size)
  {
    if constexpr (!is_simple<T>()) {
      /* Run destructors. */
      for (int i = 0; i < size; i++) {
        old[i].~T();
      }
    }

    if (old && old != static_storage()) {
      alloc::release(static_cast<void *>(old));
    }
  }

  flatten_inline T *static_storage()
  {
    return reinterpret_cast<T *>(static_storage_);
  }

  T *data_ = nullptr;
  size_t size_ = 0;
  size_t capacity_ = 0;
#ifdef WASM
  // pad to eight bytes
  // since pointer size is 4 on WASM.
  int _pad;
#endif
  uint8_t static_storage_[static_size * sizeof(T)];
};

} // namespace litestl::util