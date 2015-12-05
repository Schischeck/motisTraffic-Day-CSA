#include "motis/realtime/handler/util.h"

#include <unordered_map>

#include "motis/realtime/realtime_schedule.h"
#include "motis/core/common/logging.h"

#include "motis/protocol/RISMessage_generated.h"

using namespace motis::ris;
using namespace motis::logging;

namespace motis {
namespace realtime {
namespace handler {

unsigned find_station_by_eva(std::string const& eva, schedule const& sched) {
  auto it = sched.eva_to_station.find(eva);
  return it == end(sched.eva_to_station) ? 0 : it->second->index;
}

unsigned find_station_by_ds100(std::string const& ds100,
                               schedule const& sched) {
  auto it = sched.ds100_to_station.find(ds100);
  return it == end(sched.ds100_to_station) ? 0 : it->second->index;
}

schedule_event ris_event_to_schedule_event(Event const& ris_event,
                                           schedule const& sched) {
  schedule_event e(
      0, ris_event.trainIndex(), ris_event.type() == EventType_Departure,
      unix_to_motistime(sched.schedule_begin_, ris_event.scheduledTime()));

  switch (ris_event.stationIdType()) {
    case StationIdType_EVA:
      e._station_index =
          find_station_by_eva(ris_event.stationId()->str(), sched);
      if (e._station_index == 0) {
        // LOG(warn) << "Station with EVA = " << ris_event.stationId()->str()
        //           << " not found";
      }
      break;
    case StationIdType_DS100:
      e._station_index =
          find_station_by_ds100(ris_event.stationId()->str(), sched);
      if (e._station_index == 0) {
        // LOG(warn) << "Station with DS100 = " << ris_event.stationId()->str()
        //           << " not found";
      }
      break;
    case StationIdType_Context: break;
  }

  return e;
}

schedule_event ris_event_to_schedule_event(motis::ris::Event const& ris_event,
                                           schedule_event const& context_event,
                                           schedule const& sched) {
  schedule_event e = ris_event_to_schedule_event(ris_event, sched);
  if (ris_event.stationIdType() == StationIdType_Context) {
    e._station_index = context_event._station_index;
  }
  return e;
}

}  // namespace handler
}  // namespace realtime
}  // namespace motis
