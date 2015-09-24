#pragma once

#include "google/dense_hash_set"

namespace motis {

template <typename Entry, typename Hash = std::hash<Entry>,
          typename Eq = std::equal_to<Entry>>
using hash_set = google::dense_hash_set<Entry, Hash, Eq>;

template <typename Entry, typename Hash, typename Eq, typename CreateFun>
auto set_get_or_create(hash_set<Entry, Hash, Eq>& s, Entry const& key,
                       CreateFun f) -> decltype(*s.find(key)) & {
  auto it = s.find(key);
  if (it != s.end()) {
    return *it;
  } else {
    return *s.insert(f()).first;
  }
}

}  // namespace motis
