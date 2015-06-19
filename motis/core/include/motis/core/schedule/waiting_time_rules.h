#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <bitset>

#include "motis/core/common/flat_matrix.h"

namespace td {

struct schedule;
class graph_loader;

class waiting_time_rules {
public:
  friend class graph_loader;

  int waiting_time_category(const std::string& train_category) const
  {
    auto it = _category_map.find(train_category);
    if (it == end(_category_map))
      return default_group;
    else
      return it->second;
  }

  inline int waiting_time_category(int family) const
  { return _family_to_wtr_category[family]; }

  inline int waiting_time(int connecting_category, int feeder_category) const
  { return _waiting_time_matrix[connecting_category][feeder_category]; }

  inline bool waits_for_other_trains(int connecting_category) const
  { return _waits_for_other_trains[connecting_category]; }

  inline bool other_trains_wait_for(int feeder_category) const
  { return _other_trains_wait_for[feeder_category]; }

  int default_group;

private:
  void add_category(int wzr_category,
                   const std::string& train_categories)
  {
    std::stringstream ss(train_categories);
    std::string train_category;

    while (std::getline(ss, train_category, ';')) {
      _category_map[train_category] = wzr_category;
    }
  }

  std::unordered_map<std::string, int> _category_map;
  std::vector<int> _family_to_wtr_category;
  flat_matrix<duration> _waiting_time_matrix;
  std::vector<bool> _waits_for_other_trains;
  std::vector<bool> _other_trains_wait_for;
};

} // namespace td

