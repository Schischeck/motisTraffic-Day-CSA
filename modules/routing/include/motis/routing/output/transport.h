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
      : from(from),
        to(to),
        con(con),
        duration(con->a_time - con->d_time),
        slot(-1),
        route_id(route_id) {}

  transport(unsigned int from, unsigned int to, unsigned int duration, int slot,
            unsigned int route_id)
      : from(from),
        to(to),
        con(nullptr),
        duration(duration),
        slot(slot),
        route_id(route_id) {}

  unsigned int from, to;
  light_connection const* con;
  unsigned int duration;
  int slot;
  unsigned int route_id;
};

}  // namespace intermediate
}  // namespace output
}  // namespace routing
}  // namespace motis