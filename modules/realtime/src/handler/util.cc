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
  if (it == std::end(sched.eva_to_station)) {
    return 0;
  } else {
    return it->second->index;
  }
}

unsigned find_station_by_ds100(std::string const& ds100,
                               schedule const& sched) {
  auto it = sched.ds100_to_station.find(ds100);
  if (it == std::end(sched.ds100_to_station)) {
    return 0;
  } else {
    return it->second->index;
  }
}

std::unordered_map<std::time_t, motis::time> time_cache;

time local_ts_to_time(std::time_t unix_ts, schedule const& sched) {
  auto it = time_cache.find(unix_ts);
  if (it != time_cache.end()) {
    return it->second;
  }
  // timestamps in messages are in the local timezone, not utc
  std::tm* t = std::localtime(&unix_ts);
  if (t == nullptr) return INVALID_TIME;

  auto schedule_begin =
      boost::posix_time::from_time_t(sched.schedule_begin_).date();
  boost::gregorian::date msg_date(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
  motis::time mt = INVALID_TIME;
  if (msg_date >= schedule_begin) {
    mt = motis::to_time((msg_date - schedule_begin).days(),
                        t->tm_hour * 60 + t->tm_min);
  }
  if (time_cache.size() > 10000) {
    time_cache.clear();
  }
  time_cache[unix_ts] = mt;
  return mt;
}

schedule_event ris_event_to_schedule_event(Event const& ris_event,
                                           schedule const& sched) {
  schedule_event e(0, ris_event.trainIndex(),
                   ris_event.type() == EventType_Departure,
                   local_ts_to_time(ris_event.scheduledTime(), sched));

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
