#pragma once

#include "motis/core/access/time_access.h"
#include "motis/core/access/trip_access.h"
#include "motis/core/conversion/event_type_conversion.h"
#include "motis/protocol/TripId_generated.h"

namespace motis {

inline trip const* from_fbs(schedule const& sched, TripId const* t) {
  return get_trip(sched, t->station_id()->str(), t->train_nr(), t->time(),
                  t->target_station_id()->str(), t->target_time(),
                  t->line_id()->str());
}

inline flatbuffers::Offset<TripId> to_fbs(schedule const& sched,
                                          flatbuffers::FlatBufferBuilder& fbb,
                                          trip const* trp) {
  auto const& p = trp->id_.primary_;
  auto const& s = trp->id_.secondary_;
  return CreateTripId(
      fbb, fbb.CreateString(sched.stations_.at(p.station_id_)->eva_nr_),
      p.train_nr_, motis_to_unixtime(sched, p.time_),
      fbb.CreateString(sched.stations_.at(s.target_station_id_)->eva_nr_),
      motis_to_unixtime(sched, s.target_time_), fbb.CreateString(s.line_id_));
}

}  // namespace motis
