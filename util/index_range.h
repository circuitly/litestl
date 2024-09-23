#pragma once
namespace litestl::util {
struct IndexRange {
  int start, size;

  IndexRange() : start(0), size(0)
  {
  }

  IndexRange(int count) : start(0), size(count)
  {
  }

  IndexRange(int a, int b) : start(a), size(b)
  {
  }

  struct iterator {
    iterator(int i) : i_(i)
    {
    }
    iterator(const iterator &b) : i_(b.i_)
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

    int operator*() const
    {
      return i_;
    }
    iterator &operator++()
    {
      i_++;

      return *this;
    }

  private:
    int i_;
  };

  iterator begin() const
  {
    return iterator(0);
  }

  iterator end() const
  {
    return iterator(start + size);
  }
};
} // namespace litestl::util
