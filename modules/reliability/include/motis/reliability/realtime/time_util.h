#pragma once

#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/time.h"

namespace motis {
namespace reliability {
namespace time_util {

enum event_type { departure, arrival };
inline motis::time get_scheduled_event_time(light_connection const& lc,
                                            unsigned int const, event_type type,
                                            schedule const&) {
  if (type == departure) return lc.d_time;
  return lc.a_time;
}

}  // namespace time_util
}  // namespace reliability
}  // namespace motis
