#pragma once

#include "motis/core/schedule/connection.h"

namespace motis {

namespace routing {
namespace output {
namespace intermediate {

struct transport {
  transport() = default;
  transport(unsigned const from, unsigned const to, light_connection const* con)
      : from_(from),
        to_(to),
        con_(con),
        duration_(con->a_time_ - con->d_time_),
        mumo_id_(0),
        mumo_price_(0) {}

  transport(unsigned const from, unsigned const to, unsigned const duration,
            int const mumo_id, unsigned const mumo_price)
      : from_(from),
        to_(to),
        con_(nullptr),
        duration_(duration),
        mumo_id_(mumo_id),
        mumo_price_(mumo_price) {}

  unsigned from_, to_;
  light_connection const* con_;
  unsigned duration_;
  int mumo_id_;
  unsigned mumo_price_;
};

}  // namespace intermediate
}  // namespace output
}  // namespace routing
}  // namespace motis
