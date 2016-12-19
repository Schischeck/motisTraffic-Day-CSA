#pragma once

#include <algorithm>
#include <vector>

namespace motis {
namespace path {

template <typename T>
void append(std::vector<T>& v1, std::vector<T> const& v2) {
  v1.insert(end(v1), begin(v2), end(v2));
}

template <typename T, typename P>
void erase_if(std::vector<T>& vec, P const& pred) {
  vec.erase(std::remove_if(begin(vec), end(vec), pred), end(vec));
}

}  // namespace path
}  // namespace motis
