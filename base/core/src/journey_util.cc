#include "motis/core/journey/journey_util.h"

#include <algorithm>

#include "motis/core/schedule/time.h"
#include "motis/core/journey/journey.h"

namespace motis {

uint16_t get_duration(journey const& journey) {
  if (journey.stops.size() > 0) {
    return (journey.stops.back().arrival.timestamp -
            journey.stops.front().departure.timestamp) /
           60;
  }
  return 0;
}
uint16_t get_transfers(journey const& journey) {
  return std::count_if(journey.stops.begin(), journey.stops.end(),
                       [](journey::stop const& s) { return s.interchange; });
}

void print_journey(journey const& j, time_t const sched_begin,
                   std::ostream& os) {
  auto format = [&](time_t t) -> std::string {
    return format_time(unix_to_motistime(sched_begin, t));
  };
  auto to_str = [&](journey::transport const& t) -> std::string {
    switch (t.type) {
      case journey::transport::PublicTransport: return t.name;
      case journey::transport::Walk: return "Walk";
      case journey::transport::Mumo: {
        std::stringstream sst;
        sst << t.mumo_type_name << "," << t.mumo_price;
        return sst.str();
      }
    }
    return "unknown";
  };

  unsigned int db_cost = 0;
  std::for_each(j.transports.begin(), j.transports.end(),
                [&](journey::transport const& t) { db_cost += t.mumo_price; });

  os << "Journey (" << j.duration << ", " << j.transfers << ", " << db_cost
     << ")\n";
  for (auto const& t : j.transports) {
    auto const& from = j.stops[t.from];
    auto const& to = j.stops[t.to];
    os << from.name << " " << format(from.departure.timestamp) << " --"
       << to_str(t) << "-> " << format(to.arrival.timestamp) << " " << to.name
       << std::endl;
  }
}

}  // namespace motis
