#pragma once

#include "motis/core/schedule/time.h"

namespace motis {

struct schedule;
struct schedule_event;
struct graph_event;

namespace ris {

struct Event;

}  // namespace ris

namespace realtime {
namespace handler {

schedule_event ris_event_to_schedule_event(motis::ris::Event const& ris_event,
                                           schedule const& sched);

schedule_event ris_event_to_schedule_event(motis::ris::Event const& ris_event,
                                           schedule_event const& context_event,
                                           schedule const& sched);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
