#pragma once

#include <cstdint>

#include "motis/core/journey/journey.h"

namespace motis {

uint16_t get_duration(journey const&);
uint16_t get_transfers(journey const&);

template <typename Journey, typename F>
void foreach_light_connection(Journey& journey, F func) {
  for (auto& transport : journey.transports) {
    if (transport.walk) {
      continue;
    }

    for (auto i = transport.from; i < transport.to; ++i) {
      func(transport, journey.stops[i], journey.stops[i + 1]);
    }
  }
}

}  // namespace motis
