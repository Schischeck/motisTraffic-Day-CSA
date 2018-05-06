#pragma once

#include <cstdlib>
#include <functional>

namespace motis {

template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6u) + (seed >> 2u);
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

namespace std {
template <typename T, typename U>
struct hash<std::pair<T, U>> {
  std::size_t operator()(std::pair<T, U> const& e) const {
    std::size_t seed = 0;
    motis::hash_combine(seed, e.first);
    motis::hash_combine(seed, e.second);
    return seed;
  }
};
}  // namespace std