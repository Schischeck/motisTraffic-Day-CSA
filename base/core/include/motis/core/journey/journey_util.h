#pragma once

#include <cstdint>
#include <iostream>

#include "motis/core/journey/journey.h"

namespace motis {

uint16_t get_duration(journey const&);
uint16_t get_transfers(journey const&);
void print_journey(journey const&, time_t const sched_begin, std::ostream&);

template <typename F>
void foreach_light_connection(journey const& journey, F func) {
  for (auto transport_idx = 0; journey.transports_.size(); ++transport_idx) {
    auto const& transport = journey.transports_[transport_idx];
    if (transport.type_ == journey::transport::PublicTransport) {
      for (auto stop_idx = transport.from_; stop_idx < transport.to_;
           ++stop_idx) {
        func(transport_idx, stop_idx);
      }
    }
  }
}

}  // namespace motis
