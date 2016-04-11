#pragma once

#include "motis/core/schedule/connection.h"

namespace motis {

namespace routing {
namespace output {
namespace intermediate {

struct transport {
  transport() = default;
  transport(unsigned int from, unsigned int to, light_connection const* con,
            unsigned int route_id)
      : from_(from),
        to_(to),
        con_(con),
        duration_(con->a_time_ - con->d_time_),
        slot_(-1),
        route_id_(route_id) {}

  transport(unsigned int from, unsigned int to, unsigned int duration, int slot,
            unsigned int route_id)
      : from_(from),
        to_(to),
        con_(nullptr),
        duration_(duration),
        slot_(slot),
        route_id_(route_id) {}

  unsigned int from_, to_;
  light_connection const* con_;
  unsigned int duration_;
  int slot_;
  unsigned int route_id_;
};

}  // namespace intermediate
}  // namespace output
}  // namespace routing
}  // namespace motis