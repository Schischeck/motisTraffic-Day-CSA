#pragma once

#include "motis/core/access/time_access.h"

namespace motis {
namespace reliability {
namespace test_util {

constexpr auto TIMEZONE_BERLIN = 120;

inline motis::time minutes_to_motis_time(uint16_t minutes) {
  auto const hh = minutes / 60;
  auto const mm = minutes % 60;
  return motis_time(hh * 100 + mm, 0, TIMEZONE_BERLIN);
}

inline std::time_t hhmm_to_unixtime(schedule const& sched, uint16_t hhmm,
                                    uint16_t day_offset = 0) {
  return unix_time(sched, hhmm, day_offset, TIMEZONE_BERLIN);
}

}  // namespace test_util
}  // namespace reliability
}  // namespace motis
