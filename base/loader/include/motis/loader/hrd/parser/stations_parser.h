#pragma once

#include <map>

#include "motis/loader/loaded_file.h"
#include "motis/loader/hrd/parser/station_meta_data_parser.h"
#include "motis/schedule-format/Station_generated.h"

namespace motis {
namespace loader {
namespace hrd {

struct intermediate_station {
  std::string name;
  int change_time;
  double lng, lat;
  std::vector<std::string> ds100;
};

std::map<int, intermediate_station> parse_stations(
    loaded_file const& station_names_file,
    loaded_file const& station_coordinates_file, station_meta_data const&);

}  // hrd
}  // loader
}  // motis
