#pragma once

#include "motis/core/schedule/timezone.h"

namespace motis {
namespace loader {

constexpr auto kAdjust = 60;

timezone create_timezone(int general_offset, int season_offset,
                         int day_idx_schedule_first_day,
                         int day_idx_schedule_last_day,
                         int day_idx_season_first_day,
                         int day_idx_season_last_day,
                         int minutes_after_midnight_season_begin,
                         int minutes_after_midnight_season_end);

time get_event_time(int day_idx, uint32_t local_time, timezone const* tz);

time get_adjusted_event_time(int day_idx, uint32_t local_time,
                             timezone const* tz);

bool is_in_season(int day_idx, int local_time, timezone const* tz);

}  // namespace loader
}  // namespace motis
