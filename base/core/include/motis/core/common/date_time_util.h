#pragma once

#include <ctime>
#include <string>

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

namespace motis {

inline std::time_t to_unix_time(boost::posix_time::ptime const& t) {
  boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
  return (t - epoch).total_seconds();
}

inline std::time_t to_unix_time(boost::gregorian::date const& date) {
  return to_unix_time(boost::posix_time::ptime(date));
}

inline std::time_t to_unix_time(int year, int month, int day) {
  return to_unix_time(boost::gregorian::date(year, month, day));
}

inline int hhmm_to_min(int const hhmm) {
  if (hhmm < 0) {
    return hhmm;
  } else {
    return (hhmm / 100) * 60 + (hhmm % 100);
  }
}

inline std::string format_unixtime(time_t const t) {
  auto const s = std::string(std::ctime(&t));
  return s.substr(0, s.size() - 1);
}

}  // namespace motis
