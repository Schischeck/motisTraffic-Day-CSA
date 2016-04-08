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

  auto station_id = id->station_id()->str();
  auto station_idx = get_station(sched, station_id)->index;
  auto service_num = id->service_num();
  auto motis_time = unix_to_motistime(sched, id->schedule_time());

  auto map_it = sched.trips.find({station_idx, service_num, motis_time});
  if (map_it == end(sched.trips)) {
    ++counter.trips_.missed_primary;

    // second try: ris may contain artificial train numbers
    map_it = sched.trips.find({station_idx, 0, motis_time});
    if (map_it == end(sched.trips)) {
      auto t = id->schedule_time();

      if (id->trip_type() == TripType_Additional) {
        // we cant guarantee to get all additional trains
        ++counter.trips_.is_additional;

      } else if (t < external_schedule_begin(sched) ||
                 t > external_schedule_end(sched)) {
        // assessments or decisions may contain trains not in the schedule range
        ++counter.trips_.primary_not_in_schedule;
      } else {
        std::cout << "fail " << station_id << " " << service_num << " " << t
                  << std::endl;
      }
      return nullptr;  // TODO
    } else {
      ++counter.trips_.fuzzy_primary;
    }
  }

  if (map_it->second.size() == 1) {
    ++counter.trips_.found;
    return nullptr;  // match
  }

  // fail
  return nullptr;  // TODO
}

}  // namespace handler
}  // namespace rt
}  // namespace motis
