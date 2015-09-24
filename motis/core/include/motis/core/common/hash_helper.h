#pragma once

#include <cstdlib>
#include <functional>

namespace motis {

template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <typename HashFun, typename T>
struct deep_ptr_hash {
  std::size_t operator()(T const* arg) const {
    if (arg == nullptr) {
      return 0;
    } else {
      HashFun f;
      return f(*arg);
    }
  }
};

template <typename T>
struct deep_ptr_eq {
  bool operator()(T const* lhs, T const* rhs) const {
    if (lhs == nullptr) {
      return rhs == nullptr;
    } else if (rhs == nullptr) {
      return lhs == nullptr;
    } else {
      return *lhs == *rhs;
    }
  }
};

}  // namespace motis
