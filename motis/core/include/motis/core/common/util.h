#pragma once

#include <memory>
#include <type_traits>

namespace motis {

#if !defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
template <typename T, typename... Args>
std::unique_ptr<T> make_unique_helper(std::false_type, Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return make_unique_helper<T>(std::is_array<T>(), std::forward<Args>(args)...);
}
#else
using std::make_unique;
#endif
  
} // namespace motis
