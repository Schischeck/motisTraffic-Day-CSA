#pragma once

#include <algorithm>
#include <iostream>
#include <string>

#include "motis/core/schedule/schedule.h"
#include "motis/core/access/error.h"
#include "motis/core/access/station_access.h"
#include "motis/core/access/time_access.h"
#include "motis/protocol/TripId_generated.h"

namespace motis {

inline trip const* get_trip(schedule const& sched, std::string const& eva_nr,
                            uint32_t const train_nr,
                            std::time_t const timestamp,
                            std::string const& target_eva_nr,
                            std::time_t const target_timestamp,
                            bool const is_arrival, std::string const& line_id) {
  auto const station_id = get_station(sched, eva_nr)->index_;
  auto const motis_time = unix_to_motistime(sched, timestamp);
  auto const primary_id = primary_trip_id(station_id, train_nr, motis_time);

  auto it =
      std::lower_bound(begin(sched.trips_), end(sched.trips_),
                       std::make_pair(primary_id, static_cast<trip*>(nullptr)));
  if (it == end(sched.trips_) || !(it->first == primary_id)) {
    throw std::system_error(access::error::service_not_found);
  }

  auto const target_station_id = get_station(sched, target_eva_nr)->index_;
  auto const target_motis_time = unix_to_motistime(sched, target_timestamp);
  for (; it != end(sched.trips_) && it->first == primary_id; ++it) {
    auto const& s = it->second->id_.secondary_;
    if (line_id == s.line_id_ && target_station_id == s.target_station_id_ &&
        target_motis_time == s.target_time_ && is_arrival == s.is_arrival_) {
      return it->second;
    }
  }

  throw std::system_error(access::error::service_not_found);
}

inline trip const* get_trip(schedule const& sched, TripId const* t) {
  return get_trip(sched, t->station_id()->str(), t->train_nr(), t->time(),
                  t->target_station_id()->str(), t->target_time(), false,
                  t->line_id()->str());
}

}  // namespace motis
