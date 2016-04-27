#pragma once

#include <vector>

namespace std {

template <typename T>
ostream& operator<<(ostream& out, vector<T> const& v) {
  auto it = begin(v);
  while (it != end(v)) {
    if (it != begin(v)) {
      out << " ";
    }
    out << *it;
    ++it;
  }
  return out;
}

}  // namespace std
