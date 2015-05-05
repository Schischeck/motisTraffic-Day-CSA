#ifndef TD_INTERVAL_MAP_H_
#define TD_INTERVAL_MAP_H_

#include <map>
#include <vector>
#include <memory>

namespace td {

class IntervalMap
{
public:
  struct Range
  {
    Range() = default;
    Range(int from, int to) : from(from), to(to) {}
    int from, to;
  };

  IntervalMap();
  ~IntervalMap();
  void addEntry(int attribute, int index);
  void addEntry(int attribute, int fromIndex, int toIndex);
  std::map<int, std::vector<Range>> getAttributeRanges();

private:
  class IntervalMapImpl;
  std::unique_ptr<IntervalMapImpl> _impl;
};

}  // namespace td

#endif  // TD_INTERVAL_MAP_H_