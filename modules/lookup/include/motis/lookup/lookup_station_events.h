#pragma once

#include <vector>

#include "motis/core/schedule/schedule.h"
#include "motis/protocol/Message_generated.h"

namespace motis {
namespace lookup {

std::vector<flatbuffers::Offset<StationEvent>> lookup_station_events(
    flatbuffers::FlatBufferBuilder&, schedule const&,
    LookupStationEventsRequest const*);

}  // namespace lookup
}  // namespace motis
