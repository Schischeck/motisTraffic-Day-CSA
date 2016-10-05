#pragma once

#include <map>
#include <string>

#include "geo/latlng.h"

#include "motis/routes/prepare/fbs/use_32bit_flatbuffers.h"
#include "motis/routes/prepare/fbs/use_64bit_flatbuffers.h"

#include "motis/routes/fbs/StopPositions_generated.h"
#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace routes {

std::map<std::string, std::vector<geo::latlng>> find_bus_stop_positions(
    motis::loader::Schedule const*, std::string const& osm_file);

flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<StopPositions>>>
write_stop_positions(flatbuffers::FlatBufferBuilder&,
                     std::map<std::string, std::vector<geo::latlng>> const&);

}  // namespace routes
}  // namespace motis