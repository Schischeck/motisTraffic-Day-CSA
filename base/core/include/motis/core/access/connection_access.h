#pragma once

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/trip.h"

namespace motis {
namespace access {

inline connection_info const& get_connection_info(schedule const& sched,
                                                  light_connection const& lcon,
                                                  trip const* trp) {
  auto const& trips = *sched.merged_trips[lcon.trips];
  if (trips.size() == 1) {
    return *lcon._full_con->con_info;
  }

  auto const it = std::find(begin(trips), end(trips), trp);
  assert(it != end(trips));
  auto const pos = std::distance(begin(trips), it);

  auto info = lcon._full_con->con_info;
  for (int i = 0; i < pos; ++i) {
    info = info->merged_with;
  }
  return *info;
}

}  // namespace access
}  // namespace motis
