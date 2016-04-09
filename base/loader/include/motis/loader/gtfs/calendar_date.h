#pragma once

#include <map>
#include <string>
#include <vector>

#include "boost/date_time/gregorian/gregorian_types.hpp"

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace gtfs {

struct date {
  enum { ADD, REMOVE } type;
  boost::gregorian::date day;
};

std::map<std::string, std::vector<date>> read_calendar_date(loaded_file);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
