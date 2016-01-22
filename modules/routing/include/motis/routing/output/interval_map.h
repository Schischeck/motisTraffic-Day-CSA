#pragma once

#include <map>
#include <vector>
#include <memory>

#include "boost/icl/interval_set.hpp"

namespace motis {
namespace routing {
namespace output {

template <typename T>
class interval_map {
public:
  struct range {
    range() = default;
    range(int from, int to) : from(from), to(to) {}
    int from, to;
  };

  typedef boost::icl::interval<int>::type interval;

  void add_entry(T attribute, int index) {
    attributes[attribute] += interval(index, index + 1);
  }

  void add_entry(T attribute, int from_index, int to_index) {
    attributes[attribute] += interval(from_index, to_index + 1);
  }

  std::map<T, std::vector<range>> get_attribute_ranges() {
    std::map<T, std::vector<range>> ranges;
    for (auto const& attr : attributes) {
      ranges[attr.first].reserve(attr.second.size());
      for (auto const& range : attr.second) {
        ranges[attr.first].emplace_back(range.lower(), range.upper() - 1);
      }
    }
    return ranges;
  }

private:
  std::map<T, boost::icl::interval_set<int>> attributes;
};

}  // namespace output
}  // namespace routing
}  // namespace motis