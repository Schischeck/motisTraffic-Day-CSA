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

  std::vector<T> to_vector() {
    return transform_to_vec(begin(elements_), end(elements_),
                            [](entry_t const& el) { return el.second; });
  }

  template <typename... Args>
  void emplace(index_t idx, Args... args) {
    auto s = std::make_pair(idx, T(std::forward<Args>(args)...));
    auto it = std::lower_bound(elements_.begin(), elements_.end(), s, cmp());
    elements_.emplace(it, std::move(s));
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
  friend const_iterator begin(flat_map const& m) { return m.begin(); }
  friend const_iterator end(flat_map const& m) { return m.end(); }
  friend iterator begin(flat_map& m) { return m.begin(); }
  friend iterator end(flat_map& m) { return m.end(); }

private:
  std::vector<entry_t> elements_;
};

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
