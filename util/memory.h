#include "util/alloc.h"
#include "util/compiler_util.h"
#include "util/vector.h"

#include <cstdio>

namespace litestl::util {
template <typename T> struct weak_ptr;

template <typename T> struct shared_ptr {
  struct data {
    int users;
    Vector<bool *> weak_ptrs;
    T *ptr;
  };

  shared_ptr() : data_(nullptr)
  {
  }

  shared_ptr(T *ptr)
  {
    data_ = alloc::New<data>("shared ptr data");
    data_->ptr = ptr;
    data_->users = 1;
  }

  shared_ptr(T &&b)
  {
    data_ = b.data_;
    b.data_ = nullptr;
  }

  shared_ptr(const shared_ptr &b)
  {
    data_ = b.data_;
    inc();
  }

  ~shared_ptr()
  {
    if (data_) {
      dec();
    }
  }

  shared_ptr &operator=(const shared_ptr &b)
  {
    if (this == &b) {
      return *this;
    }

    data_check_dec(b);

    data_ = b.data_;
    inc();

    return *this;
  }

  shared_ptr &operator=(shared_ptr &&b)
  {
    if (this == &b) {
      return *this;
    }

    data_check_dec(b);

    data_ = b.data_;
    b.data_ = nullptr;

    return *this;
  }

  T *operator*()
  {
    return data_->ptr;
  }

  T *operator->()
  {
    return data_->ptr;
  }

  T *get()
  {
    return data_->ptr;
  }

private:
  friend struct weak_ptr<T>;

  shared_ptr(data *data_ptr)
  {
    data_ = data_ptr;
    inc();
  }

  template <typename Ref> void data_check_dec(Ref b)
  {
    if (!data_ || data_ == b.data_) {
      return;
    }

    dec();
  }

  void inc()
  {
    data_->users++;
    // printf("inc: %d (%p)\n", data_->users, data_->ptr);
  }

  void dec()
  {
    data_->users--;
    // printf("dec: %d (%p)\n", data_->users, data_->ptr);

    if (data_->users == 0) {
      for (bool *weak : data_->weak_ptrs) {
        *weak = false;
      }

      alloc::Delete<T>(data_->ptr);
      alloc::Delete<data>(data_);
      data_ = nullptr;
    }
  }

  data *data_ = nullptr;
};

template <typename T> struct weak_ptr {
  using SharedPtr = shared_ptr<T>;
  bool expired()
  {
    return !exists_;
  }

  SharedPtr lock()
  {
    return SharedPtr(ptr_);
  }

  weak_ptr()
  {
  }

  weak_ptr(const weak_ptr &b)
  {
    exists_ = b.exists_;
    ptr_ = b.ptr_;

    if (ptr_) {
      ptr_->weak_ptrs.append(&exists_);
    }
  }

  weak_ptr &operator=(const weak_ptr &b)
  {
    if (this == &b) {
      return *this;
    }

    remove();
    exists_ = b.exists_;
    ptr_ = b.ptr_;

    if (ptr_) {
      ptr_->weak_ptrs.append(&exists_);
    }

    return *this;
  }

  weak_ptr &operator=(weak_ptr &&b)
  {
    if (this == &b) {
      return *this;
    }

    remove();
    exists_ = b.exists_;
    ptr_ = b.ptr_;
    if (ptr_) {
      ptr_->weak_ptrs.append(&exists_);
    }

    b.remove();
    b.exists_ = false;
    b.ptr_ = nullptr;

    return *this;
  }

  weak_ptr &operator=(weak_ptr &b)
  {
    if (this == &b) {
      return *this;
    }

    remove();

    exists_ = b.exists_;
    ptr_ = b.ptr_;
    if (ptr_) {
      ptr_->weak_ptrs.append(&exists_);
    }

    return *this;
  }

  weak_ptr(const SharedPtr &ptr)
  {
    ptr_ = ptr.data_;
    exists_ = ptr_ != nullptr;

    if (ptr_) {
      ptr_->weak_ptrs.append(&exists_);
    }
  }

  ~weak_ptr()
  {
    remove();
  }

private:
  void remove()
  {
    if (ptr_) {
      ptr_->weak_ptrs.remove(&exists_);
    }
  }

  bool exists_ = false;
  typename SharedPtr::data *ptr_ = nullptr;
};

} // namespace litestl::util
