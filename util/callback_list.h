#include "function.h"
#include "vector.h"

#include <functional>

namespace litestl::util {
template <typename Fn> struct CallbackList {
  using FuncType = function<Fn>;

  struct FuncOwnerPair {
    FuncType func;
    void *owner;
  };

  using iterator = Vector<FuncOwnerPair>::iterator;

  template <typename... Args> void exec(Args... args)
  {
    for (FuncOwnerPair &cb : callbacks_) {
      cb.func(std::forward<Args>(args)...);
    }
  }

  template <typename OwnerType> void add(FuncType func, OwnerType owner)
  {
    callbacks_.append({func, static_cast<void *>(owner)});
  }

  template <typename OwnerType> bool remove(OwnerType owner)
  {
    /* TODO: get Vector working with <algorithm>. */

    int cur = 0;
    void *owner_p = static_cast<void *>(owner);

    bool found = false;

    for (int i = 0; i < callbacks_.size(); i++) {
      if (callbacks_[i].owner != owner_p) {
        if (cur != i) {
          callbacks_[cur] = std::move(callbacks_[i]);
        }

        cur++;
      } else {
        found = true;
      }
    }

    if (cur != callbacks_.size()) {
      callbacks_.resize(cur);
    }

    return found;
  }

  iterator begin()
  {
    return callbacks_.begin();
  }
  iterator end()
  {
    return callbacks_.end();
  }

private:
  Vector<FuncOwnerPair> callbacks_;
};
} // namespace litestl::util