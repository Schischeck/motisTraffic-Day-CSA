#ifndef TD_INTERVAL_MAP_H_
#define TD_INTERVAL_MAP_H_

#include <map>
#include <vector>
#include <memory>

namespace td {

class interval_map {
public:
  struct range {
    range() = default;
    range(int from, int to) : from(from), to(to) {}
    int from, to;
  };

  interval_map();
  ~interval_map();
  void add_entry(int attribute, int index);
  void add_entry(int attribute, int from_index, int to_index);
  std::map<int, std::vector<range>> get_attribute_ranges();

private:
  class interval_map_impl;
  std::unique_ptr<interval_map_impl> _impl;
};

}  // namespace td

#endif  // TD_INTERVAL_MAP_H_
