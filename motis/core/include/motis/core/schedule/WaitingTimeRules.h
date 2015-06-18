#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <bitset>

#include "motis/core/common/flat_matrix.h"

namespace td {

struct Schedule;
class GraphLoader;

class WaitingTimeRules {
public:
  friend class GraphLoader;

  int waitingTimeCategory(const std::string& trainCategory) const
  {
    auto it = _categoryMap.find(trainCategory);
    if (it == end(_categoryMap))
      return defaultGroup;
    else
      return it->second;
  }

  inline int waitingTimeCategory(int family) const
  { return _familyToWtrCategory[family]; }

  inline int waitingTime(int connectingCategory, int feederCategory) const
  { return _waitingTimeMatrix[connectingCategory][feederCategory]; }

  inline bool waitsForOtherTrains(int connectingCategory) const
  { return _waitsForOtherTrains[connectingCategory]; }

  inline bool otherTrainsWaitFor(int feederCategory) const
  { return _otherTrainsWaitFor[feederCategory]; }

  int defaultGroup;

private:
  void addCategory(int wzrCategory,
                   const std::string& trainCategories)
  {
    std::stringstream ss(trainCategories);
    std::string trainCategory;

    while (std::getline(ss, trainCategory, ';')) {
      _categoryMap[trainCategory] = wzrCategory;
    }
  }

  std::unordered_map<std::string, int> _categoryMap;
  std::vector<int> _familyToWtrCategory;
  flat_matrix<Duration> _waitingTimeMatrix;
  std::vector<bool> _waitsForOtherTrains;
  std::vector<bool> _otherTrainsWaitFor;
};

} // namespace td

