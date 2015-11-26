#include "motis/core/schedule/timezone.h"

#include <cassert>
#include <algorithm>

namespace motis {

timezone::timezone(int general_offset)
    : general_offset_(general_offset),
      season_({INVALID_TIME, INVALID_TIME, INVALID_TIME}) {}

int num_days(int schedule_begin_idx, int schedule_end_idx,
             int season_event_idx) {
  return std::max(
      std::min(schedule_end_idx, season_event_idx) - schedule_begin_idx, 0);
}

timezone::timezone(int general_offset, int season_offset,
                   int schedule_begin_idx, int schedule_end_idx,
                   int season_begin_idx, int season_end_idx,  //
                   int minutes_begin, int minutes_end)
    : general_offset_(general_offset),
      season_({season_offset,
               to_time(num_days(schedule_begin_idx, schedule_end_idx,
                                season_begin_idx),
                       MINUTES_A_DAY + minutes_begin - general_offset),
               to_time(num_days(schedule_begin_idx, schedule_end_idx,
                                season_end_idx),
                       MINUTES_A_DAY + minutes_end - season_offset)}) {}

time timezone::to_motis_time(int num_days, int minutes_after_midnight) const {
  auto const local_motis_time =
      to_time(num_days, MINUTES_A_DAY + minutes_after_midnight);
  auto const is_in_season =
      season_.begin != INVALID_TIME &&
      season_.begin + general_offset_ <= local_motis_time &&
      local_motis_time <= season_.end + season_.offset;
  return local_motis_time - (is_in_season ? season_.offset : general_offset_);
}

std::time_t timezone::to_local_time(std::time_t schedule_begin,
                                    time motis_time) const {
  auto const is_in_season = season_.begin != INVALID_TIME &&
                            season_.begin <= motis_time &&
                            motis_time <= season_.end;
  auto const active_offset = is_in_season ? season_.offset : general_offset_;
  return motis_to_unixtime(schedule_begin,
                           motis_time - MINUTES_A_DAY + active_offset);
}
}
