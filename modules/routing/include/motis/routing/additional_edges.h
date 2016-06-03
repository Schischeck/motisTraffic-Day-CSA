#pragma once

#include <vector>

#include "motis/protocol/RoutingRequest_generated.h"

namespace motis {

class edge;
struct schedule;

namespace routing {

std::vector<edge> create_additional_edges(
    flatbuffers::Vector<flatbuffers::Offset<AdditionalEdgeWrapper>> const*,
    schedule const&, unsigned const destination_station_index);

}  // namespace routing
}  // namespace motis
