#pragma once

#include <cstdint>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "motis/core/journey/journey.h"

namespace motis {

uint16_t get_duration(journey const&);
uint16_t get_transfers(journey const&);
void print_journey(journey const&, time_t const sched_begin, std::ostream&);

template <typename F>
void foreach_light_connection(journey const& j, F func) {
  for (unsigned int stop_idx = 0; stop_idx + 1 < j.stops_.size(); ++stop_idx) {
    auto const transport =
        std::find_if(j.transports_.begin(), j.transports_.end(),
                     [stop_idx](journey::transport const& tr) {
                       return stop_idx >= tr.from_ && stop_idx < tr.to_;
                     });
    if (transport == j.transports_.end()) {
      throw std::runtime_error("transport not found");
    }
    if (!transport->is_walk_) {
      func(stop_idx, std::distance(j.transports_.begin(), transport));
    }
  }
}

}  // namespace motis
