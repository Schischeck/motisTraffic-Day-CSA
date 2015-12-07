#pragma once

#include <cinttypes>
#include <climits>
#include <sstream>
#include <string>
#include <iomanip>

#define MINUTES_A_DAY 1440

namespace motis {

typedef uint16_t time;
typedef uint16_t duration;

constexpr unsigned short INVALID_TIME = USHRT_MAX;
constexpr unsigned int SCHEDULE_OFFSET = MINUTES_A_DAY * 60;

inline time to_motis_time(int day_index, int minutes_after_midnight) {
  // +1 for schedule offset
  return (1 + day_index) * MINUTES_A_DAY + minutes_after_midnight;
}

inline time to_motis_time(int day_index, int hours, int minutes) {
  return to_motis_time(day_index, hours * 60 + minutes);
}

inline std::string format_time(time t) {
  if (t == INVALID_TIME) return "INVALID";

  int day = t / MINUTES_A_DAY;
  int minutes = t % MINUTES_A_DAY;

  std::ostringstream out;
  out << std::setw(2) << std::setfill('0') << (minutes / 60) << ":"
      << std::setw(2) << std::setfill('0') << (minutes % 60) << "." << day;

  return out.str();
}

inline std::time_t motis_to_unixtime(std::time_t schedule_begin, time t) {
  return schedule_begin + t * 60;
}

inline time unix_to_motistime(std::time_t schedule_begin, std::time_t t) {
  if (t < schedule_begin) {
    return INVALID_TIME;
  }
  return (t - schedule_begin) / 60;
}

}  // namespace motis
