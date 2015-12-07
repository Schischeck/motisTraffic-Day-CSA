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
    season_begin = to_time(
        day_idx(day_idx_schedule_first_day, day_idx_schedule_last_day,
                day_idx_season_first_day),
        MINUTES_A_DAY + minutes_after_midnight_season_begin - general_offset);
  }

  time season_end = INVALID_TIME - season_offset;
  if (day_idx_season_last_day <= day_idx_schedule_last_day) {
    season_end = to_time(
        day_idx(day_idx_schedule_first_day, day_idx_schedule_last_day,
                day_idx_season_last_day),
        MINUTES_A_DAY + minutes_after_midnight_season_end - season_offset);
  }

  return {general_offset, {season_offset, season_begin, season_end}};
}

time compute_event_time(int first_day, int day, int local_time,
                        timezone const* tz) {
  return tz ? tz->to_motis_time(day - first_day, local_time)
            : (day - first_day + 1) * MINUTES_A_DAY + local_time;
}

}  // namspace loader
}  // namespace motis
