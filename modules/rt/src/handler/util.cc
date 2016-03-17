#include "motis/rt/handler/reroute_handler.h"

#include "motis/core/access/trip_access.h"
#include "motis/core/schedule/schedule.h"
#include "motis/protocol/RISMessage_generated.h"
#include "motis/rt/handler/context.h"
#include "motis/rt/handler/util.h"

using namespace motis::ris;

namespace motis {
namespace rt {
namespace handler {

trip const* get_trip(context& ctx, TripId const* id) {
  if (id->base()->stationIdType() != StationIdType_EVA ||
      id->targetStationIdType() != StationIdType_EVA) {
    ++ctx.stats.ds100;
    return nullptr;  // TODO
  }

  auto const& sched = ctx.sched;

  auto eva_nr = id->base()->stationId()->str();
  auto station_id = get_station(sched, eva_nr)->index;
  auto train_nr = id->base()->trainIndex();
  auto motis_time = unix_to_motistime(sched, id->base()->scheduledTime());

  auto map_it = sched.trips.find({station_id, train_nr, motis_time});
  if (map_it == end(sched.trips)) {

    ++ctx.stats.missed_primary;
    return nullptr;  // TODO
  }

  auto target_eva_nr = id->targetStationId()->str();
  auto target_station_id = get_station(sched, target_eva_nr)->index;
  auto target_motis_time = unix_to_motistime(sched, id->targetScheduledTime());
  auto is_arrival = (id->base()->type() == EventType_Arrival);
  auto line_id = id->base()->lineId()->str();

  auto vec = map_it->second;
  auto vec_it = std::find_if(begin(vec), end(vec), [&](trip const* t) {
    auto const& s = t->id.secondary;
    return line_id == s.line_id && target_station_id == s.target_station_id &&
           target_motis_time == s.target_time && is_arrival == s.is_arrival;
  });

  if (vec_it == end(vec)) {
    ++ctx.stats.missed_secondary;

    return nullptr;  // TODO
  }

  ++ctx.stats.found_trips;

  // return *vec_it;
  return nullptr;  // TODO
}

}  // namespace handler
}  // namespace rt
}  // namespace motis
