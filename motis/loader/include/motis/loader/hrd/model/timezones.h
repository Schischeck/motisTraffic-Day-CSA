#pragma once

#include <vector>
#include <map>
#include <memory>

namespace motis {
namespace loader {
namespace hrd {

struct timezone_entry {
  timezone_entry(int general_gmt_offset, int seasonal_gmt_offset,
                 int season_begin_idx, int season_begin_time,
                 int season_end_idx, int season_end_time)
      : general_gmt_offset(general_gmt_offset),
        seasonal_gmt_offset(seasonal_gmt_offset),
        season_begin_idx(season_begin_idx),
        season_begin_time(season_begin_time),
        season_end_idx(season_end_idx),
        season_end_time(season_end_time) {}

  int const general_gmt_offset;  // in minutes
  int const seasonal_gmt_offset;  // in minutes
  int const season_begin_idx;  // bitfield index (closed)
  int const season_begin_time;  // minutes after midnight
  int const season_end_idx;  // bitfield index (closed)
  int const season_end_time;  // minutes after midnight
};

struct timezones {
  std::map<int, timezone_entry*> eva_num_to_tz_entry;
  std::vector<std::unique_ptr<timezone_entry>> timezone_entries_;
};

}  // hrd
}  // loader
}  // motis
