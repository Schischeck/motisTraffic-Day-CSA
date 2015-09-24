#pragma once

#include <ctime>

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

namespace motis {

inline std::time_t to_unix_time(int year, int month, int day) {
  boost::posix_time::ptime t(boost::gregorian::date(year, month, day));
  boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
  return (t - epoch).total_seconds();
}

}  // namespace motis
