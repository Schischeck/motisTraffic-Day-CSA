#include "motis/loader/gtfs/services.h"

#include "boost/date_time/gregorian/gregorian.hpp"

namespace greg = boost::gregorian;

namespace motis {
namespace loader {
namespace gtfs {

greg::date bound_date(std::map<std::string, calendar> const& base, bool first) {
  if (base.empty()) {
    return {1970, 1, 1};
  }

  if (first) {
    return std::min_element(begin(base), end(base),
                            [](std::pair<std::string, calendar> const& lhs,
                               std::pair<std::string, calendar> const& rhs) {
                              return lhs.second.first_day <
                                     rhs.second.first_day;
                            })
        ->second.first_day;
  } else {
    return std::max_element(begin(base), end(base),
                            [](std::pair<std::string, calendar> const& lhs,
                               std::pair<std::string, calendar> const& rhs) {
                              return lhs.second.last_day < rhs.second.last_day;
                            })
        ->second.last_day;
  }
}

bitfield calendar_to_bitfield(greg::date const& start, calendar const& c) {
  auto first = std::min(start, c.first_day);
  auto last =
      std::min(start + greg::days(BIT_COUNT), c.last_day + greg::days(1));

  bitfield traffic_days;
  int bit = (first - start).days();
  for (auto d = first; d != last; d += greg::days(1), ++bit) {
    traffic_days.set(bit, c.week_days.test(d.day_of_week()));
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

services traffic_days(
    std::map<std::string, calendar> const& base,
    std::map<std::string, std::vector<date>> const& exceptions) {
  services s;
  s.first_day = bound_date(base, true);
  s.last_day = bound_date(base, false);

  for (auto const& base_calendar : base) {
    s.traffic_days[base_calendar.first] = make_unique<bitfield>(
        calendar_to_bitfield(s.first_day, base_calendar.second));
  }

  for (auto const& exception : exceptions) {
    for (auto const& day : exception.second) {
      add_exception(s.first_day, day, *s.traffic_days[exception.first].get());
    }
  }

  return s;
}

}  // namespace gtfs
}  // namespace loader
}  // namespace motis
