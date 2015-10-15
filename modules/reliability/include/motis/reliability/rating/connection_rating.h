#pragma once

#include <vector>

#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/probability_distribution.h"

namespace motis {
struct schedule;
namespace routing {
struct Connection;
}

namespace reliability {
struct start_and_travel_distributions;
namespace distributions_container {
struct precomputed_distributions_container;
}

namespace rating {

struct connection_element : distributions_calculator::common::queue_element {
  connection_element(unsigned int const departure_stop_idx, node const* from,
                     node const* to, light_connection const* light_connection,
                     unsigned short const light_connection_idx,
                     bool const is_first_route_node)
      : distributions_calculator::common::queue_element(
            from, to, light_connection, light_connection_idx,
            is_first_route_node),
        departure_stop_idx_(departure_stop_idx) {}
  unsigned int const departure_stop_idx_;
  inline unsigned int arrival_stop_idx() const {
    return departure_stop_idx_ + 1;
  }
};

struct rating_element {
  rating_element(unsigned int departure_stop_idx)
      : departure_stop_idx_(departure_stop_idx) {}
  probability_distribution departure_distribution_;
  probability_distribution arrival_distribution_;
  unsigned int const departure_stop_idx_;
  inline unsigned int arrival_stop_idx() const {
    return departure_stop_idx_ + 1;
  }
};

std::vector<rating_element> rate(
    routing::Connection const*, schedule const&,
    distributions_container::precomputed_distributions_container const&,
    start_and_travel_distributions const&);

}  // namespace rating
}  // namespace reliability
}  // namespace motis
