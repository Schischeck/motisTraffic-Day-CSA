#pragma once

#include <ctime>

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

namespace motis {

inline std::time_t to_unix_time(boost::posix_time::ptime const& t) {
  static boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
  return (t - epoch).total_seconds();
}

inline std::time_t to_unix_time(boost::gregorian::date const& date) {
  return to_unix_time(boost::posix_time::ptime(date));
}

inline std::time_t to_unix_time(int year, int month, int day) {
  return to_unix_time(boost::gregorian::date(year, month, day));
}

}  // namespace motis
