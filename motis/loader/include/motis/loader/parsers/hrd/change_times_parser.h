#pragma once

#include <map>

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace hrd {

struct change_times {
  change_times(loaded_file const& infotext_file);

  int get_interchange_time(int eva_num) const;

  static const char* MINCT;
  std::map<int, int> eva_num_to_interchange_time_;
};

}  // hrd
}  // loader
}  // motis
