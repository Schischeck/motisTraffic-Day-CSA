#include "motis/loader/timezone_util.h"

#include "motis/core/schedule/time.h"
#include "motis/core/schedule/timezone.h"

#include <algorithm>

#include "parser/util.h"

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
    season_begin = to_motis_time(
        day_idx(day_idx_schedule_first_day, day_idx_schedule_last_day,
                day_idx_season_first_day),
        minutes_after_midnight_season_begin - general_offset);
  }

  time season_end = INVALID_TIME - season_offset;
  if (day_idx_season_last_day <= day_idx_schedule_last_day) {
    season_end = to_motis_time(
        day_idx(day_idx_schedule_first_day, day_idx_schedule_last_day,
                day_idx_season_last_day),
        minutes_after_midnight_season_end - season_offset);
  }

  return {general_offset, {season_offset, season_begin, season_end}};
}

time get_event_time(int day_idx, uint32_t local_time, timezone const* tz) {
  return tz != nullptr ? tz->to_motis_time(day_idx, local_time)
                       : to_motis_time(day_idx, local_time);
}

time get_adjusted_event_time(int day_idx, uint32_t local_time,
                             timezone const* tz) {
  auto const t = get_event_time(day_idx, local_time, tz);
  if (t.valid()) {
    return t;
  }

  auto const adjusted = get_event_time(day_idx, local_time + kAdjust, tz);
  verify(adjusted.valid(), "adjusted needs to be valid");
  return adjusted;
}

bool is_in_season(int day_idx, int minutes_after_midnight, timezone const* tz) {
  auto const minutes_after_schedule_begin =
      time(day_idx, minutes_after_midnight);
  // = motis::to_motis_time(day_idx, minutes_after_midnight);  Scheint ein
  // fehler zu sein TODO: Prüfen ob dem so ist!
  return tz->season_.begin_ != INVALID_TIME &&
         tz->season_.begin_ + tz->general_offset_ <=
             minutes_after_schedule_begin &&
         minutes_after_schedule_begin <= tz->season_.end_ + tz->season_.offset_;
}

}  // namespace loader
}  // namespace motis
