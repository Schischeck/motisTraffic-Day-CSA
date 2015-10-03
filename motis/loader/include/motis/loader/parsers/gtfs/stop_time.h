#pragma once

#include <map>
#include <tuple>
#include <string>

#include "parser/cstr.h"

#include "motis/loader/loaded_file.h"
#include "motis/loader/parsers/gtfs/flat_map.h"

namespace motis {
namespace loader {
namespace gtfs {

struct stop_time {
  stop_time() = default;
  stop_time(std::string stop, std::string headsign, int arr_time,
            bool out_allowed, int dep_time, bool in_allowed)
      : stop(stop),
        headsign(headsign),
        arr({arr_time, out_allowed}),
        dep({dep_time, in_allowed}) {}

  struct ev {
    int time;
    bool in_out_allowed;
  };

  std::string stop;
  std::string headsign;
  ev arr, dep;
};

std::map<std::string, flat_map<stop_time>> read_stop_times(loaded_file);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
