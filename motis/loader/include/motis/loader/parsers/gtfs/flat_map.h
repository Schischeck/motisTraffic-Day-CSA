#pragma once

#include <algorithm>
#include <vector>
#include <utility>

namespace motis {
namespace loader {
namespace gtfs {

template <typename T>
struct flat_map {
  typedef std::size_t index_t;
  typedef std::pair<index_t, T> entry_t;
  typedef typename std::vector<entry_t>::iterator iterator;
  typedef typename std::vector<entry_t>::const_iterator const_iterator;

  struct cmp {
    bool operator()(entry_t const& lhs, entry_t const& rhs) {
      return lhs.first < rhs.first;
    }
  };

  template <typename... Args>
  void emplace(index_t idx, Args... args) {
    auto s = std::make_pair(idx, T(std::forward<Args>(args)...));
    auto it = std::lower_bound(elements_.begin(), elements_.end(), s, cmp());
    elements_.emplace(it, s);
  }

  T& operator[](index_t idx) {
    auto s = std::make_pair(idx, T());
    auto it = std::lower_bound(elements_.begin(), elements_.end(), s, cmp());
    if (it == elements_.end() || it->first != idx) {
      it = elements_.emplace(it, s);
    }
    return it->second;
  }

  iterator begin() { return elements_.begin(); }
  iterator end() { return elements_.end(); }
  const_iterator begin() const { return elements_.begin(); }
  const_iterator end() const { return elements_.end(); }

private:
  std::vector<entry_t> elements_;
};

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
