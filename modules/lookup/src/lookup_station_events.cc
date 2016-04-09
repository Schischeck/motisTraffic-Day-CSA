#include "motis/lookup/lookup_station_events.h"

#include "motis/core/access/edge_access.h"
#include "motis/core/access/service_access.h"
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

Offset<StationEvent> make_event(FlatBufferBuilder& fbb, schedule const& sched,
                                light_connection const* lcon,
                                unsigned const station_index,
                                int const route_id, bool is_dep) {
  auto trip_ids = make_trip_ids(fbb, sched, lcon);

  auto const& fcon = *lcon->_full_con;
  auto const& info = *fcon.con_info;

  auto const type = is_dep ? EventType_Departure : EventType_Arrival;

  auto const& time = is_dep ? lcon->d_time : lcon->a_time;
  auto const sched_time = get_schedule_time(sched, station_index, info.train_nr,
                                            is_dep, time, route_id);

  std::string dir;
  if (info.dir_ != nullptr) {
    dir = *info.dir_;
  } else {
    // XXX what happens with multiple trips?!
    auto trp = sched.merged_trips[lcon->trips]->at(0);
    dir = sched.stations[trp->id.secondary.target_station_id]->name;
  }

  auto const& track = sched.tracks[is_dep ? fcon.d_platform : fcon.a_platform];
  auto const& service_name = get_service_name(sched, &info);

  return CreateStationEvent(
      fbb, fbb.CreateVector(trip_ids), type, info.train_nr,
      fbb.CreateString(info.line_identifier), motis_to_unixtime(sched, time),
      motis_to_unixtime(sched, sched_time), fbb.CreateString(dir),
      fbb.CreateString(service_name), fbb.CreateString(track));
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
  // TODO filter (departures and arrivals)
  // TODO sort by time

  std::vector<Offset<StationEvent>> events;
  for (auto const& route_node : station_node->get_route_nodes()) {
    auto const& route_id = route_node->_route;

    for (auto const& edge : route_node->_incoming_edges) {
      foreach_arrival_in(*edge, begin, end, [&](light_connection const* lcon) {
        events.push_back(
            make_event(fbb, sched, lcon, station_index, route_id, false));
      });
    }

    for (auto const& edge : route_node->_edges) {
      foreach_departure_in(edge, begin, end, [&](light_connection const* lcon) {
        events.push_back(
            make_event(fbb, sched, lcon, station_index, route_id, true));
      });
    }
  }
  return events;
}

}  // namespace lookup
}  // namespace motis
