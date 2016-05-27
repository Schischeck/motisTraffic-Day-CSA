#pragma once

#include "motis/core/schedule/connection.h"

namespace motis {

namespace routing {
namespace output {
namespace intermediate {

struct transport {
  transport() = default;
  transport(unsigned int from, unsigned int to, light_connection const* con,
            uint16_t slot)
      : from_(from),
        to_(to),
        con_(con),
        duration_(con->a_time_ - con->d_time_),
        slot_(slot) {}

  transport(unsigned int from, unsigned int to, unsigned int duration,
            uint8_t slot)
      : from_(from), to_(to), con_(nullptr), duration_(duration), slot_(slot) {}

  unsigned from_, to_;
  light_connection const* con_;
  unsigned duration_;
  uint8_t slot_;
};

}  // namespace intermediate
}  // namespace output
}  // namespace routing
}  // namespace motis
