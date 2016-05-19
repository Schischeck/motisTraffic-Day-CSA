#pragma once

#include "motis/core/schedule/event.h"
#include "motis/core/schedule/time.h"

namespace motis {

enum class timestamp_reason { SCHEDULE, IS, FORECAST, PROPAGATION, REPAIR };

struct delay_info {
  explicit delay_info(schedule_event ev)
      : schedule_event_(ev),
        forecast_time_(motis::INVALID_TIME),
        current_time_(ev.schedule_time_),
        reason_(timestamp_reason::SCHEDULE) {}

  schedule_event schedule_event_;
  time forecast_time_, current_time_;
  timestamp_reason reason_;
};

}  // namespace motis
