#pragma once

#include "motis/core/schedule/schedule.h"

#include "motis/rt/bfs.h"

namespace motis {
namespace rt {

void seperate_trip(schedule const& sched, trip const* trp) {
  auto const first_e = trp->edges_->at(0).get_edge();
  auto const first_dep = ev_key{first_e, trp->lcon_idx_, event_type::DEP};
  for (auto const& ev : trip_bfs(first_dep, bfs_direction::BOTH)) {
    if (ev.ev_type_ == event_type::ARR) {
      continue;
    }
  }
}

}  // namespace rt
}  // namespace motis
