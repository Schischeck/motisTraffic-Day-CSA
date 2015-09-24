#pragma once

#include <map>

#include "motis/loader/loaded_file.h"
#include "motis/loader/parsers/hrd/station_meta_data_parser.h"
#include "motis/schedule-format/Station_generated.h"

namespace motis {
namespace loader {
namespace hrd {

struct intermediate_station {
  std::string name;
  int change_time;
  double lng, lat;
};

std::map<int, intermediate_station> parse_stations(
    loaded_file station_names_file, loaded_file station_coordinates_file,
    station_meta_data const&);

}  // hrd
}  // loader
}  // motis
