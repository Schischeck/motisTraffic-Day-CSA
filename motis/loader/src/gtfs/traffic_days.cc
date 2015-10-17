#include "motis/loader/parsers/gtfs/traffic_days.h"

#include "boost/date_time/gregorian/gregorian.hpp"

namespace greg = boost::gregorian;

namespace motis {
namespace loader {
namespace gtfs {

greg::date first_date(std::map<std::string, calendar> const& base) {
  if (base.empty()) {
    return {1970, 1, 1};
  }
  return std::min_element(begin(base), end(base),
                          [](std::pair<std::string, calendar> const& lhs,
                             std::pair<std::string, calendar> const& rhs) {
                            return lhs.second.first_day < rhs.second.first_day;
                          })->second.first_day;
}

bitfield calendar_to_bitfield(greg::date const& start, calendar const& c) {
  auto first = std::min(start, c.first_day);
  auto last =
      std::min(start + greg::days(BIT_COUNT), c.last_day + greg::days(1));

  bitfield traffic_days;
  int bit = (first - start).days();
  for (auto d = first; d != last; d += greg::days(1), ++bit) {
    traffic_days.set(c.week_days[d.day_of_week()]);
  }
  return traffic_days;
}

void add_exception(greg::date const& start, date const& exception,
                   bitfield& b) {
  auto day_idx = (exception.day - start).days();
  if (day_idx < 0 || day_idx >= static_cast<int>(b.size())) {
    return;
  }
  b.set(day_idx, exception.type == date::ADD);
}

std::map<std::string, bitfield> traffic_days(
    std::map<std::string, calendar> const& base,
    std::map<std::string, std::vector<date>> const& exceptions) {
  std::map<std::string, bitfield> services;

  auto start = first_date(base);

  for (auto const& base_calendar : base) {
    services[base_calendar.first] =
        calendar_to_bitfield(start, base_calendar.second);
  }

  for (auto const& exception : exceptions) {
    for (auto const& day : exception.second) {
      add_exception(start, day, services[exception.first]);
    }
  }

  return services;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
