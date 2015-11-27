#include "motis/core/journey/journey_util.h"

#include <algorithm>

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

} // namespace motis
