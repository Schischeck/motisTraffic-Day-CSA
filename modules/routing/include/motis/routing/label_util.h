#pragma once

#include <cassert>
#include <iostream>

namespace motis {
namespace label_util {
namespace detail {
/* note: all parameters are minutes after midnight */
inline bool is_night(uint16_t const t, uint16_t const night_begin,
                     uint16_t const night_end) {
  return t >= night_begin && t <= night_end;
}
}

inline uint16_t night_travel_duration(uint16_t const begin_time,
                                      uint16_t const end_time,
                                      bool const hotel_edge,
                                      uint16_t const night_begin,
                                      uint16_t const night_end) {
  if (hotel_edge) {
    return 0;
  }
  assert(begin_time <= end_time);
  assert(end_time - begin_time < 1440);

  uint16_t const midnight_arrival_day = end_time - (end_time % 1440);
  uint16_t const begin_intersection = std::max(
      begin_time, static_cast<uint16_t>(midnight_arrival_day + night_begin));
  uint16_t const end_intersection = std::min(
      end_time, static_cast<uint16_t>(midnight_arrival_day + night_end));

  if (begin_intersection < end_intersection) {
    return end_intersection - begin_intersection;
  }
  return 0;
}
}
}
