#pragma once

#include <string>
#include <map>
#include <vector>

#include "motis/loader/bitfield.h"
#include "motis/loader/parsers/gtfs/calendar.h"
#include "motis/loader/parsers/gtfs/calendar_date.h"

namespace motis {
namespace loader {
namespace gtfs {

std::map<std::string, bitfield> traffic_days(
    std::map<std::string, calendar> const&,
    std::map<std::string, std::vector<date>> const&);

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
