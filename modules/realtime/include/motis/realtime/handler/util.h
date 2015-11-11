#pragma once

#include "motis/core/schedule/time.h"

namespace motis {

struct schedule;

namespace ris {

struct Event;

}  // namespace ris

namespace realtime {

class schedule_event;
class graph_event;

namespace handler {

time local_ts_to_time(std::time_t unix_ts, schedule const& sched);

schedule_event ris_event_to_schedule_event(motis::ris::Event const& ris_event,
                                           schedule const& sched);

schedule_event ris_event_to_schedule_event(motis::ris::Event const& ris_event,
                                           schedule_event const& context_event,
                                           schedule const& sched);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
