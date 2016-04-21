#pragma once

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"
#include "motis/lookup/error.h"

// naming: find_xyz returns nullptr on miss
// naming: get_xyz throws on miss

namespace motis {
namespace lookup {

inline time get_schedule_time(schedule const& sched, unsigned station_index,
                              uint32_t const train_nr, bool const is_departure,
                              time const t, int const route_id) {
  graph_event evt{station_index, train_nr, is_departure, t, route_id};
  auto it = sched.graph_to_delay_info.find(evt);
  if (it != end(sched.graph_to_delay_info)) {
    return it->second->_schedule_event._schedule_time;
  } else {
    return t;
  }
}

// TODO actually this should in schedule_access.h somewhere in core
//      but what about the error?
inline station_node* get_station_node(schedule const& sched,
                                      std::string const& eva_nr) {
  auto it = sched.eva_to_station.find(eva_nr);
  if (it == end(sched.eva_to_station)) {
    throw boost::system::system_error(error::station_not_found);
  }
  return sched.station_nodes[it->second->index].get();
}

// simple case -> each route node has one route edge (no merge split)
inline edge* find_outgoing_route_edge(node* node) {
  for (auto& edge : node->_edges) {
    if (edge.type() == edge::ROUTE_EDGE) {
      return &edge;
    }
  }
  return nullptr;
}

inline edge* get_outgoing_route_edge(node* node) {
  auto res = find_outgoing_route_edge(node);
  if (res == nullptr) {
    throw boost::system::system_error(error::route_edge_not_found);
  }
  return res;
}

}  // namespace lookup
}  // namespace motis
