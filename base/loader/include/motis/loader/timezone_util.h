#pragma once

#include "motis/core/schedule/timezone.h"

namespace motis {
namespace loader {

timezone create_timezone(int general_offset, int season_offset,
                         int day_idx_schedule_first_day,
                         int day_idx_schedule_last_day,
                         int day_idx_season_first_day,
                         int day_idx_season_last_day,
                         int minutes_after_midnight_season_begin,
                         int minutes_after_midnight_season_end);

time get_event_time(int day_idx, int local_time, timezone const* tz);

std::pair<time, time> get_event_times(int day_idx,  //
                                      int prev_arr_motis_time,
                                      int curr_dep_local_time,
                                      int curr_arr_local_time,
                                      timezone const* tz_dep,
                                      timezone const* tz_arr, int& offset);

}  // namspace loader
}  // namespace motis
