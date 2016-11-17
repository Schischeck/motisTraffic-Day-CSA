#pragma once

#include <algorithm>
#include <vector>

namespace motis {

template <typename T, typename P>
void erase_if(std::vector<T>& vec, P const& pred) {
  vec.erase(std::remove_if(begin(vec), end(vec), pred), end(vec));
}

}  // namespace motis
