#pragma once

#include <map>

namespace motis {

template <typename K, typename V, typename CreateFun>
V& get_or_create(std::map<K, V>& m, K const& key, CreateFun f) {
  auto it = m.find(key);
  if (it != end(m)) {
    return it->second;
  } else {
    return m[key] = f();
  }
}

} // namespace motis
