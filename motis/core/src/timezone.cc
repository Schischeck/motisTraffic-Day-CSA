#include "motis/core/schedule/timezone.h"

#include "motis/core/schedule/time.h"

namespace motis {

time timezone::to_motis_time(int day_idx, int local_time) {
  auto const active_offset = general_offset_;

  auto const day_offset = local_time / MINUTES_A_DAY;
  auto const ev_time_last_day = local_time % MINUTES_A_DAY;
  auto const active_day = day_idx + day_offset;
  if (season_.first_day_idx != season::NOT_SET &&
      (!(season_.first_day_idx <= active_day) ||
       season_.begin_time <= ev_time_last_day) &&
      (!(active_day <= season_.last_day_idx) ||
       ev_time_last_day <= season_.end_time)) {
    active_offset = season_.offset;
  }

  return MINUTES_A_DAY + (day_idx * MINUTES_A_DAY + local_time + active_offset);
}
}
