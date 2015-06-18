#ifndef TD_TDTIME_H_
#define TD_TDTIME_H_

#include <cinttypes>
#include <climits>
#include <sstream>
#include <string>
#include <iomanip>

#define MINUTES_A_DAY 1440

namespace td
{

typedef uint16_t Time;
typedef uint16_t Duration;

constexpr unsigned short INVALID_TIME = USHRT_MAX;

inline Time toTime(int dayIndex, int minutes)
{ return dayIndex * MINUTES_A_DAY + minutes; }

inline std::string formatTime(Time time)
{
  if (time == INVALID_TIME)
    return "INVALID";

  int day = time / MINUTES_A_DAY;
  int minutes = time % MINUTES_A_DAY;

  std::ostringstream out;
  out << std::setw(2) << std::setfill('0') << (minutes / 60) << ":"
      << std::setw(2) << std::setfill('0') << (minutes % 60) << "."
      << day;

  return out.str();
}

}  // namespace td

#endif  // TD_TDTIME_H_

