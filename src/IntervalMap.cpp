#include "IntervalMap.h"

#include <boost/icl/interval_set.hpp>

namespace td {

class IntervalMap::IntervalMapImpl
{
public:
  typedef boost::icl::interval<int>::type interval;

  void addEntry(int attribute, int index)
  { attributes[attribute] += interval(index, index + 1); }

  void addEntry(int attribute, int fromIndex, int toIndex)
  { attributes[attribute] += interval(fromIndex, toIndex + 1); }

  std::map<int, std::vector<IntervalMap::Range>> getAttributeRanges()
  {
    std::map<int, std::vector<IntervalMap::Range>> ranges;
    for (auto const& attr : attributes)
    {
      ranges[attr.first].reserve(attr.second.size());
      for (auto const& range : attr.second)
        ranges[attr.first].emplace_back(range.lower(), range.upper() - 1);
    }
    return ranges;
  }

private:
  std::map<int, boost::icl::interval_set<int>> attributes;
};

IntervalMap::IntervalMap() : _impl(new IntervalMapImpl()) {}
IntervalMap::~IntervalMap() {}

void IntervalMap::addEntry(int attribute, int index)
{ _impl->addEntry(attribute, index); }

void IntervalMap::addEntry(int attribute, int fromIndex, int toIndex)
{ _impl->addEntry(attribute, fromIndex, toIndex); }

std::map<int, std::vector<IntervalMap::Range>> IntervalMap::getAttributeRanges()
{ return _impl->getAttributeRanges(); }

}  // namespace td