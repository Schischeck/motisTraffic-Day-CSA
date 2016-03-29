#include "motis/lookup/lookup_station_events.h"

#include "motis/core/access/edge_access.h"
#include "motis/core/access/time_access.h"
#include "motis/lookup/util.h"

using namespace flatbuffers;

namespace motis {
namespace lookup {

std::vector<Offset<TripId>> make_trip_ids(FlatBufferBuilder& fbb,
                                          schedule const& sched,
                                          light_connection const* lcon) {
  std::vector<Offset<TripId>> trip_ids;
  for (auto const& trp : *sched.merged_trips[lcon->trips]) {
    auto const& pri = trp->id.primary;
    auto const& eva_nr = sched.stations[pri.station_id]->eva_nr;
    auto const& train_nr = pri.train_nr;
    auto const& timestamp = motis_to_unixtime(sched, pri.time);

    auto const& sec = trp->id.secondary;
    auto const& target_eva_nr = sched.stations[sec.target_station_id]->eva_nr;
    auto const& target_timestamp = motis_to_unixtime(sched, sec.target_time);
    auto const& type = sec.is_arrival ? EventType_Arrival : EventType_Departure;
    auto const& line_id = sec.line_id;

    trip_ids.push_back(CreateTripId(fbb, fbb.CreateString(eva_nr), train_nr,
                                    timestamp, fbb.CreateString(target_eva_nr),
                                    target_timestamp, type,
                                    fbb.CreateString(line_id)));
  }
  return trip_ids;
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
        auto trip_ids = make_trip_ids(fbb, sched, lcon);

        auto const& info = lcon->_full_con->con_info;
        auto const& a_time = lcon->a_time;
        auto schedule_a_time = get_schedule_time(
            sched, station_index, info->train_nr, false, a_time, route_id);

        events.push_back(CreateStationEvent(
            fbb, fbb.CreateVector(trip_ids), EventType_Arrival, info->train_nr,
            fbb.CreateString(info->line_identifier),
            motis_to_unixtime(sched, a_time),
            motis_to_unixtime(sched, schedule_a_time)));
      });
    }

    for (auto const& edge : route_node->_edges) {
      foreach_departure_in(edge, begin, end, [&](light_connection const* lcon) {
        auto trip_ids = make_trip_ids(fbb, sched, lcon);

        auto const& info = lcon->_full_con->con_info;
        auto const& d_time = lcon->d_time;
        auto schedule_d_time = get_schedule_time(
            sched, station_index, info->train_nr, true, d_time, route_id);

        events.push_back(CreateStationEvent(
            fbb, fbb.CreateVector(trip_ids), EventType_Departure,
            info->train_nr, fbb.CreateString(info->line_identifier),
            motis_to_unixtime(sched, d_time),
            motis_to_unixtime(sched, schedule_d_time)));
      });
    }
  };
  return events;
}

}  // namespace lookup
}  // namespace motis
