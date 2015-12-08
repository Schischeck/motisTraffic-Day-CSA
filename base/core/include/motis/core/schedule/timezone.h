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

  inline time to_motis_time(int day_idx, int minutes_after_midnight) const {
    printf("  %d %d\t%02d:%02d.%d", day_idx, minutes_after_midnight,
           minutes_after_midnight / 60, minutes_after_midnight % 60, day_idx);

    auto const minutes_after_schedule_begin =
        motis::to_motis_time(day_idx, minutes_after_midnight);

    auto const is_in_season =
        season_.begin != INVALID_TIME &&
        season_.begin + general_offset_ <= minutes_after_schedule_begin &&
        minutes_after_schedule_begin <= season_.end + season_.offset;
    auto is_invalid_time =
        is_in_season &&
        minutes_after_schedule_begin < season_.begin + general_offset_ + 60;

    if (is_in_season) {
      printf(" in_season");
    }
    if (is_invalid_time) {
      printf(" invalid");
    }
    printf("\n");

    if (is_invalid_time) {
      return INVALID_TIME;
    } else {
      return minutes_after_schedule_begin -
             (is_in_season ? season_.offset : general_offset_);
    }
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
