#pragma once

#include <vector>

#include "motis/protocol/RoutingRequest_generated.h"

namespace motis {
struct edge;
struct schedule;
namespace routing {
std::vector<edge> create_hotel_edges(
    flatbuffers::Vector<flatbuffers::Offset<HotelEdge>> const*,
    schedule const&);
}
}
