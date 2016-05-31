#pragma once

#include <cassert>
#include <algorithm>

#include "motis/core/schedule/time.h"

namespace motis {
namespace routing {
namespace late_connections_util {

auto constexpr NIGHT_BEGIN = 1380; /* 23:00 GMT */
auto constexpr NIGHT_END = 240; /* 04:00 GMT */

inline uint16_t night_travel_duration(time const travel_begin,
                                      time const travel_end,
                                      uint16_t const night_begin = NIGHT_BEGIN,
                                      uint16_t const night_end = NIGHT_END) {
  auto intersection = [](uint16_t const begin1, uint16_t const end1,
                         uint16_t const begin2, uint16_t const end2) {
    assert(begin1 < end1);
    assert(begin2 < end2);
    assert(begin1 < MINUTES_A_DAY);
    assert(end1 < MINUTES_A_DAY);
    assert(begin2 < MINUTES_A_DAY);
    assert(end2 < MINUTES_A_DAY);
    auto const b = std::max(begin1, begin2);
    auto const e = std::min(end1, end2);
    return static_cast<uint16_t>(std::max(0, e - b));
  };

  if (travel_begin == travel_end || night_begin == night_end) {
    return 0;
  }
  assert(travel_begin <= travel_end);
  assert(travel_end - travel_begin < MINUTES_A_DAY);
  assert(night_begin < MINUTES_A_DAY);
  assert(night_end < MINUTES_A_DAY);

  uint16_t const tb = travel_begin % MINUTES_A_DAY;
  uint16_t const te = travel_end % MINUTES_A_DAY;

  if (night_begin < night_end) {
    if (tb < te) {
      return intersection(tb, te, night_begin, night_end);
    } else if (te > night_begin) {
      return te - night_begin;
    }
  } else if (tb < te) {
    return (te - tb) - intersection(tb, te, night_end, night_begin);
  } else {
    auto const offset = MINUTES_A_DAY - std::min(night_begin, tb);
    return intersection((tb + offset) % MINUTES_A_DAY,
                        (te + offset) % MINUTES_A_DAY,
                        (night_begin + offset) % MINUTES_A_DAY,
                        (night_end + offset) % MINUTES_A_DAY);
  }

  return 0;
}

}  // namespace late_connections_util
}  // namespace routing
}  // namespace motis
