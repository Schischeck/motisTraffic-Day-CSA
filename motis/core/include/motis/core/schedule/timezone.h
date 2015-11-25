#pragma once

#include <ctime>

namespace motis {

struct season {
  static int const NOT_SET = -1;
  int first_day_idx;
  int begin_time;
  int last_day_idx;
  int end_time;
  int offset;
};

struct timezone {
  time to_motis_time(int, int);
  std::time_t to_local_time(time motis_time, std::time_t sched_begin);

  int general_offset_;
  season season_;
};
}
