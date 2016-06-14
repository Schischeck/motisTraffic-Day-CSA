#pragma once

#include <set>

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/event.h"

namespace motis {
namespace rt {

enum class bfs_direction { FORWARD, BACKWARD, BOTH };

std::set<edge const*> route_bfs(ev_key const& k, bfs_direction const dir);

}  // namespace rt
}  // namespace motis
