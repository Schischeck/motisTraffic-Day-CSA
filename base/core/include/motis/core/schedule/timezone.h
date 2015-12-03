#pragma once

#include <ctime>

#include "motis/core/schedule/time.h"

namespace motis {

struct season {
  int offset;
  time begin;
  time end;
};

struct timezone {
  explicit timezone(int general_offset)
      : general_offset_(general_offset),
        season_({INVALID_TIME, INVALID_TIME, INVALID_TIME}) {}

  timezone(int general_offset, season s)
      : general_offset_(general_offset), season_(s) {}

  inline bool is_invalid_time(time actual) const {
    if (season_.begin != INVALID_TIME) {
      auto const distanceLowerBound = actual - season_.begin;
      auto const distanceUpperBound = season_.end - actual;

      // motis time t is within 1 hour...
      // ...hour after season begin, or
      // ...hour before season end
      return (0 <= distanceLowerBound && distanceLowerBound <= 1) ||
             (0 <= distanceUpperBound && distanceUpperBound <= 1);
    }
    return false;
  }

  inline time to_motis_time(int day_idx, int minutes_after_midnight) const {
    auto const local_motis_time =
        to_time(day_idx, MINUTES_A_DAY + minutes_after_midnight);
    auto const is_in_season =
        season_.begin != INVALID_TIME &&
        season_.begin + general_offset_ <= local_motis_time &&
        local_motis_time <= season_.end + season_.offset;
    return local_motis_time - (is_in_season ? season_.offset : general_offset_);
  }

  inline std::time_t to_local_time(std::time_t schedule_begin,
                                   time motis_time) const {
    auto const is_in_season = season_.begin != INVALID_TIME &&
                              season_.begin <= motis_time &&
                              motis_time <= season_.end;
    auto const active_offset = is_in_season ? season_.offset : general_offset_;
    return motis_to_unixtime(schedule_begin,
                             motis_time - MINUTES_A_DAY + active_offset);
  }

  int const general_offset_;
  season const season_;
};
}
