#pragma once

#include <map>
#include <vector>

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

struct station_meta_data {
  struct footpath {
    int from_eva_num, to_eva_num, duration;
  };

  int get_station_change_time(int eva_num) const;

  static const char* MINCT;
  std::map<int, int> station_change_times_;
  std::vector<footpath> footpaths_;
  std::map<parser::cstr, int> ds100_to_eva_num_;
};

void parse_station_meta_data(loaded_file const& infotext_file,
                             station_meta_data&);

}  // hrd
}  // loader
}  // motis
