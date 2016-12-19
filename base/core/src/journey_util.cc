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
                       [](journey::stop const& s) { return s.exit_; }) -
         1;
}

}  // namespace motis
