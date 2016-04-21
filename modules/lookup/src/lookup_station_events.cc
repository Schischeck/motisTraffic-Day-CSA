#include "motis/lookup/lookup_station_events.h"

#include "motis/core/access/edge_access.h"
#include "motis/core/access/service_access.h"
#include "motis/core/access/station_access.h"
#include "motis/core/access/time_access.h"
#include "motis/lookup/util.h"

using namespace flatbuffers;

namespace motis {
namespace lookup {

std::vector<Offset<TripId>> make_trip_ids(FlatBufferBuilder& fbb,
                                          schedule const& sched,
                                          light_connection const* lcon) {
  std::vector<Offset<TripId>> trip_ids;
  for (auto const& trp : *sched.merged_trips_[lcon->trips_]) {
    auto const& pri = trp->id_.primary_;
    auto const& eva_nr = sched.stations_[pri.station_id_]->eva_nr_;
    auto const& train_nr = pri.train_nr_;
    auto const& timestamp = motis_to_unixtime(sched, pri.time_);

    auto const& sec = trp->id_.secondary_;
    auto const& target_eva_nr =
        sched.stations_[sec.target_station_id_]->eva_nr_;
    auto const& target_timestamp = motis_to_unixtime(sched, sec.target_time_);
    auto const& type =
        sec.is_arrival_ ? EventType_Arrival : EventType_Departure;
    auto const& line_id = sec.line_id_;

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

  auto const& fcon = *lcon->full_con_;
  auto const& info = *fcon.con_info_;

  auto const type = is_dep ? EventType_Departure : EventType_Arrival;

  auto const& time = is_dep ? lcon->d_time_ : lcon->a_time_;
  auto const sched_time = get_schedule_time(
      sched, station_index, info.train_nr_, is_dep, time, route_id);

  std::string dir;
  if (info.dir_ != nullptr) {
    dir = *info.dir_;
  } else {
    // XXX what happens with multiple trips?!
    auto trp = sched.merged_trips_[lcon->trips_]->at(0);
    dir = sched.stations_[trp->id_.secondary_.target_station_id_]->name_;
  }

  auto const& track =
      sched.tracks_[is_dep ? fcon.d_platform_ : fcon.a_platform_];
  auto const& service_name = get_service_name(sched, &info);

  return CreateStationEvent(
      fbb, fbb.CreateVector(trip_ids), type, info.train_nr_,
      fbb.CreateString(info.line_identifier_), motis_to_unixtime(sched, time),
      motis_to_unixtime(sched, sched_time), fbb.CreateString(dir),
      fbb.CreateString(service_name), fbb.CreateString(track));
}

std::vector<Offset<StationEvent>> lookup_station_events(
    FlatBufferBuilder& fbb, schedule const& sched,
    LookupStationEventsRequest const* req) {
  if (sched.schedule_begin_ > req->end() ||
      sched.schedule_end_ < req->begin()) {
    throw std::system_error(error::not_in_period);
  }

  auto station_node = get_station_node(sched, req->eva_nr()->str());
  auto station_index = station_node->id_;

  auto begin = unix_to_motistime(sched, req->begin());
  auto end = unix_to_motistime(sched, req->end());

  // TODO(sebastian) include events with schedule_time in the interval (but time
  // outside)
  // TODO(sebastian) filter (departures and arrivals)
  // TODO(sebastian) sort by time

  std::vector<Offset<StationEvent>> events;
  for (auto const& route_node : station_node->get_route_nodes()) {
    auto const& route_id = route_node->route_;

    for (auto const& edge : route_node->incoming_edges_) {
      foreach_arrival_in(*edge, begin, end, [&](light_connection const* lcon) {
        events.push_back(
            make_event(fbb, sched, lcon, station_index, route_id, false));
      });
    }

    for (auto const& edge : route_node->edges_) {
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
