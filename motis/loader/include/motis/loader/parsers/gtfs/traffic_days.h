#pragma once

#include <string>
#include <map>
#include <vector>

#include "boost/date_time/gregorian/gregorian.hpp"

#include "motis/loader/bitfield.h"
#include "motis/loader/parsers/gtfs/calendar.h"
#include "motis/loader/parsers/gtfs/calendar_date.h"

namespace motis {
namespace loader {
namespace gtfs {

struct services {
  boost::gregorian::date first_day, last_day;
  std::map<std::string, bitfield> traffic_days;
};

services traffic_days(std::map<std::string, calendar> const&,
                      std::map<std::string, std::vector<date>> const&);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
