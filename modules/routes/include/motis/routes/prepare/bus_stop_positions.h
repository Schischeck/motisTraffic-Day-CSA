#pragma once

#include <map>
#include <string>

#include "geo/latlng.h"

#include "motis/routes/prepare/fbs/use_64bit_flatbuffers.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace routes {

std::map<std::string, std::vector<geo::latlng>> find_bus_stop_positions(
    motis::loader::Schedule const*, std::string const& osm_file);

}  // namespace routes
}  // namespace motis