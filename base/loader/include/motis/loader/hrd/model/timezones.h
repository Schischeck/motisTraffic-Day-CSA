#pragma once

#include <map>
#include <memory>
#include <vector>

#include "boost/optional.hpp"

#include "parser/util.h"

namespace motis {
namespace loader {
namespace hrd {

struct season_entry {
  season_entry(int const gmt_offset, int const first_day_idx,
               int const last_day_idx, int const season_begin_time,
               int const season_end_time)
      : gmt_offset_(gmt_offset),
        first_day_idx_(first_day_idx),
        last_day_idx_(last_day_idx),
        season_begin_time_(season_begin_time),
        season_end_time_(season_end_time) {}

  int gmt_offset_;  // in minutes
  int first_day_idx_;  // bitfield index (closed)
  int last_day_idx_;  // bitfield index (closed)
  int season_begin_time_;  // minutes after midnight
  int season_end_time_;  // minutes after midnight
};

struct timezone_entry {
  timezone_entry(int general_gmt_offset, boost::optional<season_entry> season)
      : general_gmt_offset_(general_gmt_offset), season_(std::move(season)) {}
  int general_gmt_offset_;  // in minutes
  boost::optional<season_entry> season_;
};

struct timezones {
  inline timezone_entry const* find(int eva_number) const {
    verify(0 <= eva_number && eva_number <= 9999999, "invalid eva number: %d",
           eva_number);

    auto it = eva_to_tze_.upper_bound(eva_number);
    verify(it != end(eva_to_tze_) || !timezone_entries_.empty(),
           "no timezone entry for eva number: %d", eva_number);
    return std::next(it, -1)->second;
  }

  std::map<int, timezone_entry*> eva_to_tze_;
  std::vector<std::unique_ptr<timezone_entry>> timezone_entries_;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
