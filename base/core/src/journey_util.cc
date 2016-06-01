#include "motis/core/journey/journey_util.h"

#include <algorithm>
#include <numeric>

#include "motis/core/schedule/time.h"
#include "motis/core/journey/journey.h"

namespace motis {

uint16_t get_duration(journey const& journey) {
  if (!journey.stops_.empty()) {
    return (journey.stops_.back().arrival_.timestamp_ -
            journey.stops_.front().departure_.timestamp_) /
           60;
  }
  return 0;
}
uint16_t get_transfers(journey const& journey) {
  return std::count_if(begin(journey.stops_), end(journey.stops_),
                       [](journey::stop const& s) { return s.interchange_; });
}

void print_journey(journey const& j, time_t const sched_begin,
                   std::ostream& os) {
  auto format = [&](time_t t) -> std::string {
    return format_time(unix_to_motistime(sched_begin, t));
  };
  auto to_str = [&](journey::transport const& t) -> std::string {
    if (!t.is_walk_) {
      return t.name_;
    } else if (t.is_walk_) {
      if (t.slot_ == 0) {
        return "Walk";
      } else {
        std::stringstream sst;
        sst << t.mumo_type_ << "," << t.mumo_price_;
        return sst.str();
      }
    }
    return "unknown";
  };

  auto db_cost =
      std::accumulate(begin(j.transports_), end(j.transports_), 0u,
                      [](auto&& acc, auto&& t) { return acc + t.mumo_price_; });

  os << "Journey (" << j.duration_ << ", " << j.transfers_ << ", " << db_cost
     << ")\n";
  for (auto const& t : j.transports_) {
    auto const& from = j.stops_[t.from_];
    auto const& to = j.stops_[t.to_];
    os << from.name_ << " " << format(from.departure_.timestamp_) << " --"
       << to_str(t) << "-> " << format(to.arrival_.timestamp_) << " "
       << to.name_ << std::endl;
  }
}

}  // namespace motis
