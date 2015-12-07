#include "motis/loader/timezone_util.h"

#include "motis/core/schedule/time.h"
#include "motis/core/schedule/timezone.h"

namespace motis {
namespace loader {

int day_idx(int day_idx_schedule_first_day, int day_idx_schedule_last_day,
            int day_idx_season_event_day) {
  return std::max(
      std::min(day_idx_schedule_last_day, day_idx_season_event_day) -
          day_idx_schedule_first_day,
      0);
}

timezone create_timezone(int general_offset, int season_offset,
                         int day_idx_schedule_first_day,
                         int day_idx_schedule_last_day,
                         int day_idx_season_first_day,
                         int day_idx_season_last_day,
                         int minutes_after_midnight_season_begin,
                         int minutes_after_midnight_season_end) {
  if (day_idx_season_last_day < day_idx_schedule_first_day ||
      day_idx_schedule_last_day < day_idx_season_first_day) {
    return timezone(general_offset);
  }

  time season_begin = 0;
  if (day_idx_schedule_first_day <= day_idx_season_first_day) {
    season_begin =
        to_time(day_idx(day_idx_schedule_first_day, day_idx_schedule_last_day,
                        day_idx_season_first_day),
                minutes_after_midnight_season_begin - general_offset);
  }

  time season_end = INVALID_TIME - season_offset;
  if (day_idx_season_last_day <= day_idx_schedule_last_day) {
    season_end =
        to_time(day_idx(day_idx_schedule_first_day, day_idx_schedule_last_day,
                        day_idx_season_last_day),
                minutes_after_midnight_season_end - season_offset);
  }

  return {general_offset, {season_offset, season_begin, season_end}};
}

time get_event_time(int day_idx, int local_time, timezone const* tz) {
  return tz ? tz->to_motis_time(day_idx, local_time)
            : to_time(day_idx, local_time);
}

std::pair<time, time> get_event_times(int day_idx,  //
                                      int prev_arr_motis_time,
                                      int curr_dep_local_time,
                                      int curr_arr_local_time,
                                      timezone const* tz_dep,
                                      timezone const* tz_arr, int& offset) {
  auto dep_motis_time =
      get_event_time(day_idx, curr_dep_local_time + offset, tz_dep);
  auto arr_motis_time =
      get_event_time(day_idx, curr_arr_local_time + offset, tz_arr);

  auto const adjustment = 60;

  while (prev_arr_motis_time > dep_motis_time ||
         dep_motis_time == INVALID_TIME) {
    offset += adjustment;
    dep_motis_time =
        get_event_time(day_idx, curr_dep_local_time + offset, tz_dep);
    arr_motis_time =
        get_event_time(day_idx, curr_arr_local_time + offset, tz_arr);
  }

  while (arr_motis_time == INVALID_TIME) {
    offset += adjustment;
    arr_motis_time =
        get_event_time(day_idx, curr_arr_local_time + offset, tz_arr);
  }

  while (dep_motis_time == INVALID_TIME) {
    offset += adjustment;
    dep_motis_time =
        get_event_time(day_idx, curr_dep_local_time + offset, tz_dep);
  }

  return std::make_pair(dep_motis_time, arr_motis_time);
}

}  // namspace loader
}  // namespace motis
