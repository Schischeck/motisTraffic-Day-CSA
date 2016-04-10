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
      return std::tie(from_eva_num, to_eva_num) <
             std::tie(rh.from_eva_num, rh.to_eva_num);
    }
    int from_eva_num;
    int to_eva_num;
    int duration;
  };

  int get_station_change_time(int eva_num) const;

  static const char* MINCT;
  std::map<int, int> station_change_times_;
  std::set<footpath> footpaths_;
  std::map<parser::cstr, int> ds100_to_eva_num_;
};

void parse_station_meta_data(loaded_file const& infotext_file,
                             loaded_file const& metabhf_file,
                             loaded_file const& metabhf_zusatz_file,
                             station_meta_data&);

}  // namespace hrd
}  // namespace loader
}  // namespace motis
