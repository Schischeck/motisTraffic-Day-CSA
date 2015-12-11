#pragma once

#include "motis/core/schedule/delay_info.h"
#include "motis/core/schedule/time.h"

namespace motis {
struct schedule_event;

namespace ris {
struct DelayMessage;
}  // namespace ris

namespace realtime {
class realtime_schedule;

namespace handler {

void handle_delay(motis::ris::DelayMessage const* msg,
                  motis::realtime::realtime_schedule& ctx);

void handle_delay(realtime_schedule& ctx, schedule_event const& evt,
                  motis::time updated_time, timestamp_reason reason);

}  // namespace handler
}  // namespace realtime
}  // namespace motis
