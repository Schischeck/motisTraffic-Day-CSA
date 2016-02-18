#pragma once

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"
#include "motis/lookup/error.h"

namespace motis {
namespace lookup {

inline std::time_t motis_to_unixtime(schedule const& schedule, time t) {
  return motis_to_unixtime(schedule.schedule_begin_, t);
}


// TODO actually this should in schedule_access.h somewhere in core
//      but what about the error?
inline station_node* find_station_node(schedule const& sched,
                                       std::string const& eva_nr) {
  auto it = sched.eva_to_station.find(eva_nr);
  if (it == end(sched.eva_to_station)) {
    throw boost::system::system_error(error::station_not_found);
  }
  return sched.station_nodes[it->second->index].get();
}

// simple case -> each route node has one route edge (no merge split)
edge* get_outgoing_route_edge(node* node) {
  for (auto& edge : node->_edges) {
    if (edge.type() == edge::ROUTE_EDGE) {
      return &edge;
    }
  }
  return nullptr;
}

}  // namespace lookup
}  // namespace motis
