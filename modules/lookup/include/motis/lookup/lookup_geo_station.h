#pragma once

#include <vector>

#include "motis/core/schedule/schedule.h"
#include "motis/lookup/station_geo_index.h"
#include "motis/protocol/Message_generated.h"

namespace motis {
namespace lookup {

flatbuffers::Offset<LookupGeoStationResponse> lookup_geo_stations_id(
    flatbuffers::FlatBufferBuilder&, station_geo_index const&, schedule const&,
    LookupGeoStationIdRequest const*);

flatbuffers::Offset<LookupGeoStationResponse> lookup_geo_stations(
    flatbuffers::FlatBufferBuilder&, station_geo_index const&,
    LookupGeoStationRequest const*);

}  // namespace lookup
}  // namespace motis
