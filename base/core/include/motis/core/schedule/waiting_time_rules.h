#pragma once

#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>

#include "motis/core/common/flat_matrix.h"
#include "motis/core/schedule/time.h"

namespace motis {

struct waiting_time_rules {
  waiting_time_rules() : default_group_(0) {}

  int waiting_time_category(const std::string& train_category) const {
    auto it = category_map_.find(train_category);
    if (it == end(category_map_)) {
      return default_group_;
    } else {
      return it->second;
    }
  }

  inline int waiting_time_category(int family) const {
    return family_to_wtr_category_[family];
  }

  inline int waiting_time(int connecting_category, int feeder_category) const {
    return waiting_time_matrix_[connecting_category][feeder_category];
  }

  inline bool waits_for_other_trains(int connecting_category) const {
    return waits_for_other_trains_[connecting_category];
  }

  inline bool other_trains_wait_for(int feeder_category) const {
    return other_trains_wait_for_[feeder_category];
  }

  int default_group_;
  std::unordered_map<std::string, int> category_map_;
  std::vector<int> family_to_wtr_category_;
  flat_matrix<duration> waiting_time_matrix_;
  std::vector<bool> waits_for_other_trains_;
  std::vector<bool> other_trains_wait_for_;
};

}  // namespace motis
