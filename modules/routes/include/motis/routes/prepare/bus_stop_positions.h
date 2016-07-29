#pragma once

#include <string>

#include "motis/routes/fbs/BusStopPosition_generated.h"
#include "motis/schedule-format/Schedule_generated.h"

namespace motis {
namespace routes {

flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<BusStopPosition>>>
find_bus_stop_positions(flatbuffers::FlatBufferBuilder&,
                        motis::loader::Schedule const*,
                        std::string const& osm_file);

}  // namespace routes
}  // namespace motis