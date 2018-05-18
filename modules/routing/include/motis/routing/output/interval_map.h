#pragma once

#include <map>
#include <memory>
#include <vector>

#include "boost/icl/interval_set.hpp"

namespace motis {
namespace routing {
namespace output {

template <typename T, typename Comparator = std::less<T>>
class interval_map {
public:
  struct range {
    range() = default;
    range(int from, int to) : from_(from), to_(to) {}
    int from_, to_;
  };

  using interval = boost::icl::interval<int>::type;

  void add_entry(T entry, int index) {
    attributes_[entry] += interval(index, index + 1);
  }

  void add_entry(T entry, int from_index, int to_index) {
    attributes_[entry] += interval(from_index, to_index + 1);
  }

  std::map<T, std::vector<range>, Comparator> get_attribute_ranges() {
    std::map<T, std::vector<range>, Comparator> ranges;
    for (auto const& attr : attributes_) {
      ranges[attr.first].reserve(attr.second.size());
      for (auto const& range : attr.second) {
        ranges[attr.first].emplace_back(range.lower(), range.upper() - 1);
      }
    }
    return ranges;
  }

private:
  std::map<T, boost::icl::interval_set<int>, Comparator> attributes_;
};

}  // namespace output
}  // namespace routing
}  // namespace motis
