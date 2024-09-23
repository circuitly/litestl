#pragma once 

#include <functional>
#include <type_traits>
#include <utility>

namespace litestl::util {
using std::function;

#if 1
/* TEST ME */

/* Stores a reference to a callable instead of a copy. */
template <typename function_base> class function_ref;
template <typename R, typename... Args> struct function_ref<R(Args...)> {
  template <typename CallImpl>
  function_ref(CallImpl &&impl)
      : callback_(callback_impl<typename std::remove_reference_t<CallImpl>>)
  {
    ptr_ = reinterpret_cast<void *>(&impl);
  }

  template <typename... Args2> R operator()(Args2... args)
  {
    return callback_(ptr_, std::forward<Args2>(args)...);
  }

private:
  template <typename CallImpl> static R callback_impl(void *ptr, Args... args)
  {
    return (*reinterpret_cast<CallImpl *>(ptr))(std::forward<Args>(args)...);
  }
  R (*callback_)(void *ptr, Args... args);

  void *ptr_;
};
#endif

} // namespace litestl::util
