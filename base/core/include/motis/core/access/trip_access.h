#pragma once

#include <iostream>
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
                            bool const is_arrival, std::string const& line_id) {
  auto station_id = get_station(sched, eva_nr)->index_;
  auto motis_time = unix_to_motistime(sched, timestamp);

  auto map_it = sched.trips_.find({station_id, train_nr, motis_time});
  if (map_it == end(sched.trips_)) {
    throw std::system_error(access::error::service_not_found);
  }

  auto target_station_id = get_station(sched, target_eva_nr)->index_;
  auto target_motis_time = unix_to_motistime(sched, target_timestamp);

  auto vec = map_it->second;
  auto vec_it = std::find_if(begin(vec), end(vec), [&](trip const* t) {
    auto const& s = t->id_.secondary_;
    return line_id == s.line_id_ && target_station_id == s.target_station_id_ &&
           target_motis_time == s.target_time_ && is_arrival == s.is_arrival_;
  });

  if (vec_it == end(vec)) {
    throw std::system_error(access::error::service_not_found);
  }

  return *vec_it;
}

}  // namespace motis
