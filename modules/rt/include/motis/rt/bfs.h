#pragma once

#include <set>

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/event.h"

namespace motis {
namespace rt {

enum class bfs_direction { FORWARD, BACKWARD, BOTH };

std::set<trip::route_edge> route_bfs(ev_key const&, bfs_direction const,
                                     bool with_through_edges = false);
std::set<ev_key> trip_bfs(ev_key const&, bfs_direction const);

}  // namespace rt
}  // namespace motis
