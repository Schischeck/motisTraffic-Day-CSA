#pragma once

#include <map>
#include <string>
#include <vector>

#include "geo/latlng.h"

namespace motis {

namespace loader {
struct Schedule;
}  // namespace loader

namespace routes {

std::map<std::string, std::vector<geo::latlng>> find_bus_stop_positions(
    motis::loader::Schedule const*, std::string const& osm_file);

}  // namespace routes
}  // namespace motis