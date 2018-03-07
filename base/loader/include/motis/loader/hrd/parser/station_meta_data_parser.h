#pragma once

#include <map>
#include <set>

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

struct station_meta_data {
  struct footpath {
    bool operator<(footpath const& rh) const {
      return std::tie(from_eva_num_, to_eva_num_) <
             std::tie(rh.from_eva_num_, rh.to_eva_num_);
    }
    int from_eva_num_;
    int to_eva_num_;
    int duration_;
  };

  struct meta_station {
    bool operator<(meta_station const& rh) const { return eva_ < rh.eva_; }
    int eva_;
    std::vector<int> equivalent_;
  };

  int get_station_change_time(int eva_num) const;

  static const char* minct_;
  std::map<int, int> station_change_times_;
  std::set<footpath> footpaths_;
  std::set<meta_station> meta_stations_;
  std::map<parser::cstr, int> ds100_to_eva_num_;
};

template <typename T>
void parse_station_meta_data(loaded_file const& infotext_file,
                             loaded_file const& metabhf_file,
                             loaded_file const& metabhf_zusatz_file,
                             station_meta_data&, T const&);

}  // namespace hrd
}  // namespace loader
}  // namespace motis
