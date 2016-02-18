#include "motis/lookup/lookup_station_events.h"

#include "motis/core/schedule/edge_access.h"
#include "motis/lookup/util.h"

using namespace flatbuffers;

namespace motis {
namespace lookup {

Offset<IdEvent> make_id_event(FlatBufferBuilder& fbb, schedule const& schedule,
                              int const& route_id, int const& position) {
  auto route_node_it = schedule.route_index_to_first_route_node.find(route_id);
  if (route_node_it == end(schedule.route_index_to_first_route_node)) {
    throw boost::system::system_error(error::id_event_not_found);
  }

  auto const& station = schedule.stations[route_node_it->get_station()->_id];
  auto const& lcon = get_outgoing_route_edge(route_node_it)->at(position);
  auto const& info = *lcon._full_con->con_info;

  auto d_time = lcon.d_time;

  auto delay_info_it = schedule.graph_to_delay_info.find(
      {route_node_it->_id, info->train_nr, true, lcon.d_time, route_id});
  if (delay_info_it != end(schedule.graph_to_delay_info)) {
    d_time = delay_info_it->_schedule_event->_schedule_time;
  }

  return CreateIdEvent(fbb, fbb.CreateString(station.eva_nr),
                       EventType_Departure, info->train_nr,
                       fbb.CreateString(info.line_identifier),
                       motis_to_unixtime(schedule, d_time));
}

std::vector<Offset<StationEvent>> lookup_station_events(
    FlatBufferBuilder& fbb, schedule const& schedule,
    LookupStationEventsRequest const* req) {
  auto station_node = find_station_node(schedule, req->eva_nr()->str());
  auto begin = unix_to_motistime(sched_begin, req->begin());
  auto end = unix_to_motistime(sched_begin, req->end());

  std::vector<Offset<StationEvent>> events;
  for (auto const& route_node : station_node->get_route_nodes()) {
    for (auto const& edge : route_node->_incoming_edges) {
      foreach_arrival_in(*edge, begin, end, [&](light_connection const* lcon) {

        auto position = std::distance(begin(edge->_m._route_edge._conns), lcon);
        auto id_event = make_id_event(edge->_route, position);

        auto const& info = *lcon._full_con->con_info;
        auto a_time = motis_to_unixtime(schedule, lcon.a_time);
        
        events.push_back(CreateStationEvent(fbb, id_event, EventType_Arrival,
                                            info.train_nr, fbb.CreateString(info.line_identifier)
                                            a_time));
      });
    }

    for (auto const& edge : route_node->_edges) {
      foreach_departure_in(edge, begin, end, [&](light_connection const* lcon) {
        auto const& info = *lcon._full_con->con_info;
        auto d_time = motis_to_unixtime(schedule, lcon.d_time);

        events.push_back(CreateStationEvent(fbb, 0, EventType_Departure,
                                            info.train_nr, d_time));
      });
    }
  };

  return events;
}

}  // namespace lookup
}  // namespace motis
