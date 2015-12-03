#pragma once

#include "google/dense_hash_map"

namespace motis {

template <typename Key, typename Value, typename Hash = std::hash<Key>,
          typename Eq = std::equal_to<Key>>
using hash_map = google::dense_hash_map<Key, Value, Hash, Eq>;

template <typename Key, typename Value, typename Hash, typename Eq,
          typename CreateFun>
auto map_get_or_create(hash_map<Key, Value, Hash, Eq>& m, Key const& key,
                       CreateFun f) -> decltype(m.find(key)->second) & {
  auto it = m.find(key);
  if (it != m.end()) {
    return it->second;
  } else {
    return m[key] = f();
  }
}

}  // namespace motis
