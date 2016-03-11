#include "motis/lookup/lookup_station_events.h"

#include "motis/core/access/edge_access.h"
#include "motis/lookup/util.h"

using namespace flatbuffers;

namespace motis {
namespace lookup {

Offset<IdEvent> make_id_event(FlatBufferBuilder& fbb, schedule const& sched,
                              int const& route_id, int const& position) {
  auto const& route_node = sched.route_index_to_first_route_node[route_id];
  auto const& station_index = route_node->get_station()->_id;
  auto const& station = sched.stations[station_index];

  auto const& route_edge = get_outgoing_route_edge(route_node);
  auto const& lcon = route_edge->_m._route_edge._conns[position];
  auto const& info = lcon._full_con->con_info;

  auto d_time = get_schedule_time(sched, station_index, info->train_nr, true,
                                  lcon.d_time, route_id);

  return CreateIdEvent(fbb, fbb.CreateString(station->eva_nr),
                       EventType_Departure, info->train_nr,
                       fbb.CreateString(info->line_identifier),
                       motis_to_unixtime(sched, d_time));
}

std::vector<Offset<StationEvent>> lookup_station_events(
    FlatBufferBuilder& fbb, schedule const& sched,
    LookupStationEventsRequest const* req) {
  if (sched.schedule_begin_ > req->end() ||
      sched.schedule_end_ < req->begin()) {
    throw boost::system::system_error(error::not_in_period);
  }

  auto station_node = get_station_node(sched, req->eva_nr()->str());
  auto station_index = station_node->_id;

  auto begin = unix_to_motistime(sched, req->begin());
  auto end = unix_to_motistime(sched, req->end());

  // TODO include events with schedule_time in the interval (but time outside)

  std::vector<Offset<StationEvent>> events;
  for (auto const& route_node : station_node->get_route_nodes()) {
    auto const& route_id = route_node->_route;

    for (auto const& edge : route_node->_incoming_edges) {
      foreach_arrival_in(*edge, begin, end, [&](light_connection const* lcon) {
        auto const* lcon_begin = edge->_m._route_edge._conns.begin();
        auto pos = std::distance(lcon_begin, lcon);
        auto id_event = make_id_event(fbb, sched, route_id, pos);

        auto const& info = lcon->_full_con->con_info;
        auto const& a_time = lcon->a_time;
        auto schedule_a_time = get_schedule_time(
            sched, station_index, info->train_nr, false, a_time, route_id);

        events.push_back(
            CreateStationEvent(fbb, id_event, EventType_Arrival, info->train_nr,
                               fbb.CreateString(info->line_identifier),
                               motis_to_unixtime(sched, a_time),
                               motis_to_unixtime(sched, schedule_a_time)));
      });
    }

    for (auto const& edge : route_node->_edges) {
      foreach_departure_in(edge, begin, end, [&](light_connection const* lcon) {
        auto pos = std::distance(edge._m._route_edge._conns.begin(), lcon);
        auto id_event = make_id_event(fbb, sched, route_id, pos);

        auto const& info = lcon->_full_con->con_info;
        auto const& d_time = lcon->d_time;
        auto schedule_d_time = get_schedule_time(
            sched, station_index, info->train_nr, true, d_time, route_id);

        events.push_back(CreateStationEvent(
            fbb, id_event, EventType_Departure, info->train_nr,
            fbb.CreateString(info->line_identifier),
            motis_to_unixtime(sched, d_time),
            motis_to_unixtime(sched, schedule_d_time)));
      });
    }
  };

  // TODO sort events (requires private member access in FlatBufferBuilder)

  return events;
}

}  // namespace lookup
}  // namespace motis
