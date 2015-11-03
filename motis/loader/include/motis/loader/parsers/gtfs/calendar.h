#pragma once

#include <bitset>
#include <string>
#include <map>

#include "boost/date_time/gregorian/gregorian_types.hpp"

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace gtfs {

struct calendar {
  std::bitset<7> week_days;
  boost::gregorian::date first_day, last_day;
};

std::map<std::string, calendar> read_calendar(loaded_file);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
