#pragma once

#include <ctime>

#include "motis/core/schedule/time.h"
#include "motis/core/schedule/timezone.h"

namespace motis {

struct season {
  int offset;
  time begin;
  time end;
};

struct timezone {
  explicit timezone(int general_offset);
  timezone(int general_offset, int season_offset,  //
           int schedule_begin_idx, int schedule_end_idx,  //
           int season_begin_idx, int season_end_idx,  //
           int minutes_begin, int minutes_end);
  time to_motis_time(int num_days, int minutes_local_time) const;
  std::time_t to_local_time(std::time_t schedule_begin, time motis_time) const;

  int general_offset_;
  season season_;
};
}
