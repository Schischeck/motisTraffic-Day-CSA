#pragma once

#include <cstdint>
#include <iostream>

#include "motis/core/journey/journey.h"

namespace motis {

uint16_t get_duration(journey const&);
uint16_t get_transfers(journey const&);
void print_journey(journey const&, time_t const sched_begin, std::ostream&);

template <typename Journey, typename F>
void foreach_light_connection(Journey& journey, F func) {
  for (auto& transport : journey.transports) {
    if (transport.type != Journey::transport::PublicTransport) {
      continue;
    }

    for (auto i = transport.from; i < transport.to; ++i) {
      func(transport, journey.stops[i], journey.stops[i + 1]);
    }
  }
}

}  // namespace motis
