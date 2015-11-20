#pragma once

#include <vector>
#include <map>
#include <memory>

#include "range/v3/utility/optional.hpp"

#include "parser/util.h"

namespace motis {
namespace loader {
namespace hrd {

struct season_entry {
  int const gmt_offset;  // in minutes
  int const first_day_idx;  // bitfield index (closed)
  int const season_begin_time;  // minutes after midnight
  int const last_day_idx;  // bitfield index (closed)
  int const season_end_time;  // minutes after midnight
};

struct timezone_entry {
  timezone_entry(int general_gmt_offset, ranges::optional<season_entry> season)
      : general_gmt_offset(general_gmt_offset), season(season) {}
  int const general_gmt_offset;  // in minutes
  ranges::optional<season_entry> season;
};

struct timezones {

  inline timezone_entry const* find(int eva_number) const {
    verify(0 <= eva_number && eva_number <= 9999999, "invalid eva number: %d",
           eva_number);

    auto it = eva_to_tze.upper_bound(eva_number);
    verify(it != end(eva_to_tze) || timezone_entries_.size() > 0,
           "no timezone entry for eva number: %d", eva_number);
    return std::next(it, -1)->second;
  }

  std::map<int, timezone_entry*> eva_to_tze;
  std::vector<std::unique_ptr<timezone_entry>> timezone_entries_;
};

}  // hrd
}  // loader
}  // motis
