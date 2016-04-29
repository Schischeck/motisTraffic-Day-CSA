#pragma once

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"
#include "motis/lookup/error.h"

#include "motis/protocol/Message_generated.h"

// naming: find_xyz returns nullptr on miss
// naming: get_xyz throws on miss

namespace motis {
namespace lookup {

inline time get_schedule_time(schedule const& sched, unsigned station_index,
                              uint32_t const train_nr, bool const is_departure,
                              time const t, int const route_id) {
  graph_event evt{station_index, train_nr, is_departure, t, route_id};
  auto it = sched.graph_to_delay_info_.find(evt);
  if (it != end(sched.graph_to_delay_info_)) {
    return it->second->schedule_event_.schedule_time_;
  } else {
    return t;
  }
}

// simple case -> each route node has one route edge (no merge split)
inline edge* find_outgoing_route_edge(node* node) {
  for (auto& edge : node->edges_) {
    if (edge.type() == edge::ROUTE_EDGE) {
      return &edge;
    }
  }
  return nullptr;
}

inline edge* get_outgoing_route_edge(node* node) {
  auto res = find_outgoing_route_edge(node);
  if (res == nullptr) {
    throw std::system_error(error::route_edge_not_found);
  }
  return res;
}

inline flatbuffers::Offset<Station> create_station(
    flatbuffers::FlatBufferBuilder& fbb, station const& s) {
  auto const pos = Position(s.lat(), s.lng());
  return CreateStation(fbb, fbb.CreateString(s.eva_nr_),
                       fbb.CreateString(s.name_), &pos);
}

}  // namespace lookup
}  // namespace motis
