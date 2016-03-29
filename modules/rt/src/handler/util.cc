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

trip const* get_trip(schedule const& sched, motis::ris::TripId const* id,
                     message_counter& counter) {
  ++counter.trips_.total;

  if (id->base()->stationIdType() != StationIdType_EVA ||
      id->targetStationIdType() != StationIdType_EVA) {
    ++counter.ds100_;
    return nullptr;  // TODO
  }

  auto eva_nr = id->base()->stationId()->str();
  auto station_id = get_station(sched, eva_nr)->index;
  auto train_nr = id->base()->trainIndex();
  auto motis_time = unix_to_motistime(sched, id->base()->scheduledTime());

  auto map_it = sched.trips.find({station_id, train_nr, motis_time});
  if (map_it == end(sched.trips)) {
    ++counter.trips_.missed_primary;

    // second try: ris may contain artificial train numbers
    map_it = sched.trips.find({station_id, 0, motis_time});
    if (map_it == end(sched.trips)) {
      auto t = id->base()->scheduledTime();

      if (id->tripType() == TripType_Additional) {
        // we cant guarantee to get all additional trains
        ++counter.trips_.is_additional;

      } else if (t < external_schedule_begin(sched) ||
                 t > external_schedule_end(sched)) {
        // assessments or decisions may contain trains not in the schedule range
        ++counter.trips_.primary_not_in_schedule;
      } else {
        std::cout << "fail1 " << eva_nr << " " << train_nr << " " << t
                  << std::endl;
      }
      return nullptr;  // TODO
    } else {
      ++counter.trips_.fuzzy_primary;
    }
  }

  if(map_it->second.size() == 1) {
    ++counter.trips_.found;
    return nullptr; // match
  }

  auto target_eva_nr = id->targetStationId()->str();
  auto target_station_id = get_station(sched, target_eva_nr)->index;
  auto target_motis_time = unix_to_motistime(sched, id->targetScheduledTime());
  auto is_arrival = (id->base()->type() == EventType_Arrival);
  auto line_id = id->base()->lineId()->str();

  // at least in assessments, the arrival flag seems to be consistently wrong
  auto vec = map_it->second;
  auto vec_it = std::find_if(begin(vec), end(vec), [&](trip const* t) {
    auto const& s = t->id.secondary;

    // maybe one has just more info than the other
    if (s.line_id.empty() || line_id.empty()) {
      return target_station_id == s.target_station_id &&
             target_motis_time == s.target_time;
    } else {
      return line_id == s.line_id && target_station_id == s.target_station_id &&
             target_motis_time == s.target_time;
      // && is_arrival == s.is_arrival;
    }

  });

  // TODO Anschlussbewertungen sometimes a wrong event type in the departure
  // event

  if (vec_it == end(vec)) {
    ++counter.trips_.missed_secondary;

    auto target_time = id->targetScheduledTime();
    if (target_time < external_schedule_begin(sched) ||
        target_time > external_schedule_end(sched)) {
      ++counter.trips_.secondary_not_in_schedule;
    } else {
      auto t = id->base()->scheduledTime();

      std::cout << "fail2 " << eva_nr << " " << train_nr << " " << t << " "
                << target_time << " . " << target_eva_nr << " . " << line_id
                << " . " << is_arrival << std::endl;
      std::cout << " -- " << std::endl;

      for (auto const& t : vec) {
        auto const& p = t->id.primary;
        auto const& s = t->id.secondary;

        std::cout << "fail2 " << sched.stations[p.station_id]->eva_nr << " "
                  << p.train_nr << " " << motis_to_unixtime(sched, p.time)
                  << " " << motis_to_unixtime(sched, s.target_time) << " . "
                  << sched.stations[s.target_station_id]->eva_nr << " . "
                  << s.line_id << " . " << s.is_arrival << std::endl;
      }

      // std::exit(1);
    }

    return nullptr;  // TODO
  }

  ++counter.trips_.found;

  // return *vec_it;
  return nullptr;  // TODO
}

}  // namespace handler
}  // namespace rt
}  // namespace motis
