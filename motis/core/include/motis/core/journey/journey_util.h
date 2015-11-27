#pragma once

#include <cstdint>

#include "motis/core/journey/journey.h"

namespace motis {

uint16_t get_duration(journey const&);
uint16_t get_transfers(journey const&);

template <typename F>
void foreach_light_connection(journey const& journey, F function) {
  for (auto const& transport : journey.transports) {
    if (transport.type != journey::transport::PublicTransport) {
      continue;
    }

    for (auto i = transport.from; i < transport.to; ++i) {
      function(transport, journey.stops[i], journey.stops[i + 1]);
    }
  }
}

}  // namespace motis
