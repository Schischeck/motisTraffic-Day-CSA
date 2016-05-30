#pragma once

#include <cassert>
#include <algorithm>

#include "motis/core/schedule/time.h"

namespace motis {
namespace routing {
namespace late_connections {

auto constexpr NIGHT_BEGIN = 60;
auto constexpr NIGHT_END = 360;

inline uint16_t night_travel_duration(uint16_t const travel_begin,
                                      uint16_t const travel_end,
                                      uint16_t const night_begin = NIGHT_BEGIN,
                                      uint16_t const night_end = NIGHT_END) {
  assert(travel_begin <= travel_end);
  assert(travel_end - travel_begin < 1440);
  assert(night_begin < 1440);
  assert(night_end < 1440);
  assert(night_end >= night_begin);

  uint16_t const travel_end_midnight = travel_end - (travel_end % 1440);
  uint16_t const begin_intersection = std::max(
      travel_begin, static_cast<uint16_t>(travel_end_midnight + night_begin));
  uint16_t const end_intersection = std::min(
      travel_end, static_cast<uint16_t>(travel_end_midnight + night_end));

  if (begin_intersection >= end_intersection) {
    return 0;
  }
  return end_intersection - begin_intersection;
}

}  // namespace late_connections
}  // namespace routing
}  // namespace motis
