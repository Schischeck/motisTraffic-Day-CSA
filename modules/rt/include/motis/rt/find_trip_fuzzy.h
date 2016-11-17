#pragma once

#include "motis/core/schedule/schedule.h"
#include "motis/core/access/trip_access.h"

#include "motis/protocol/RISMessage_generated.h"

namespace motis {
namespace rt {

inline trip const* find_trip_fuzzy(schedule const& sched,
                                   ris::IdEvent const* id) {
  auto const station = find_station(sched, id->station_id()->str());
  if (station == nullptr) {
    return nullptr;
  }

  auto const motis_time = unix_to_motistime(sched, id->schedule_time());
  if (motis_time == INVALID_TIME) {
    return nullptr;
  }

  trip const* trp;
  trp = find_trip(
      sched, primary_trip_id{station->index_, id->service_num(), motis_time});
  if (trp != nullptr) {
    return trp;
  }

  trp = find_trip(sched, primary_trip_id{station->index_, 0, motis_time});
  if (trp != nullptr) {
    return trp;
  }

  return nullptr;
}

}  // namespace rt
}  // namespace motis
