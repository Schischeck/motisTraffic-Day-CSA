#pragma once

#include <algorithm>
#include <string>

#include "motis/core/schedule/schedule.h"
#include "motis/core/access/error.h"
#include "motis/core/access/station_access.h"
#include "motis/core/access/time_access.h"

namespace motis {

inline trip const* get_trip(schedule const& sched, std::string const& eva_nr,
                            uint32_t const train_nr,
                            std::time_t const timestamp,
                            std::string const& target_eva_nr,
                            std::time_t const target_timestamp,
                            std::string const& line_id) {
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
        target_motis_time == s.target_time_) {
      return it->second;
    }
  }

  throw std::system_error(access::error::service_not_found);
}

inline trip const* find_trip(schedule const& sched, primary_trip_id id) {
  auto it = std::lower_bound(begin(sched.trips_), end(sched.trips_),
                             std::make_pair(id, static_cast<trip*>(nullptr)));
  if (it != end(sched.trips_) && it->first == id) {
    return it->second;
  }
  return nullptr;
}

}  // namespace motis
