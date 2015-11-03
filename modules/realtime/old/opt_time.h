#pragma once

#include <ctime>
#include <string>
#include <iostream>

#include "boost/lexical_cast.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/c_local_time_adjustor.hpp"

class opt_time {
public:
  std::time_t unix_timestamp() const {
    if (ptime_.is_not_a_date_time()) {
      return 0;
    }
    struct std::tm t;
    t.tm_sec = ptime_.time_of_day().seconds();
    t.tm_min = ptime_.time_of_day().minutes();
    t.tm_hour = ptime_.time_of_day().hours();
    t.tm_mday = ptime_.date().day();
    t.tm_mon = ptime_.date().month() - 1;
    t.tm_year = ptime_.date().year() - 1900;
    t.tm_isdst = -1;
    return std::mktime(&t);

    //    static boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1,
    //    1));
    //    boost::posix_time::time_duration diff(ptime_ - epoch);
    //    return diff.ticks() / diff.ticks_per_second();
  }

  operator std::time_t() const { return unix_timestamp(); }

  operator boost::posix_time::ptime() const { return ptime_; }

  boost::posix_time::ptime ptime_;
};

std::istream& operator>>(std::istream& in, opt_time& t);
