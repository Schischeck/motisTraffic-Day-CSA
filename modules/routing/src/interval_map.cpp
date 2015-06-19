#include "motis/routing/interval_map.h"

#include "boost/icl/interval_set.hpp"

namespace td {

class interval_map::interval_map_impl {
public:
  typedef boost::icl::interval<int>::type interval;

  void add_entry(int attribute, int index) {
    attributes[attribute] += interval(index, index + 1);
  }

  void add_entry(int attribute, int from_index, int to_index) {
    attributes[attribute] += interval(from_index, to_index + 1);
  }

  std::map<int, std::vector<interval_map::range>> get_attribute_ranges() {
    std::map<int, std::vector<interval_map::range>> ranges;
    for (auto const& attr : attributes) {
      ranges[attr.first].reserve(attr.second.size());
      for (auto const& range : attr.second)
        ranges[attr.first].emplace_back(range.lower(), range.upper() - 1);
    }
    return ranges;
  }

private:
  std::map<int, boost::icl::interval_set<int>> attributes;
};

interval_map::interval_map() : _impl(new interval_map_impl()) {}
interval_map::~interval_map() {}

void interval_map::add_entry(int attribute, int index) {
  _impl->add_entry(attribute, index);
}

void interval_map::add_entry(int attribute, int from_index, int to_index) {
  _impl->add_entry(attribute, from_index, to_index);
}

std::map<int, std::vector<interval_map::range>>
interval_map::get_attribute_ranges() {
  return _impl->get_attribute_ranges();
}

}  // namespace td
