#pragma once

#include <string>
#include <map>
#include <vector>

#include "motis/loader/loaded_file.h"

namespace motis {
namespace loader {
namespace gtfs {

struct date {
  enum { ADD, REMOVE } type;
  int day, month, year;
};

std::map<std::string, std::vector<date>> read_calendar_date(loaded_file);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
