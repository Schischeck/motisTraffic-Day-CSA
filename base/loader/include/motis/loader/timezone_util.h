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

std::pair<time, time> get_event_times(int day_idx,  //
                                      time prev_arr_motis_time,
                                      uint32_t curr_dep_local_time,
                                      uint32_t curr_arr_local_time,
                                      timezone const* tz_dep,
                                      timezone const* tz_arr, bool& adjusted);

}  // namespace loader
}  // namespace motis
