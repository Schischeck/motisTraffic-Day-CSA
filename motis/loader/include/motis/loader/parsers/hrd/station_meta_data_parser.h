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

  int get_interchange_time(int eva_num) const;

  static const char* MINCT;
  std::map<int, int> normal_change_times_;
  std::vector<footpath> footpaths_;
};

void parse_station_meta_data(loaded_file const& infotext_file,
                             station_meta_data&);

}  // hrd
}  // loader
}  // motis
