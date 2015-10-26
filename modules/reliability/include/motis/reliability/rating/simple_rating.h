#pragma once

#include <string>
#include <vector>

#include "motis/reliability/probability_distribution.h"

namespace motis {
struct schedule;
namespace routing {
struct Connection;
}

namespace reliability {
struct start_and_travel_distributions;
namespace rating {
namespace simple_rating {

enum rating_type { Cancellation, Interchange };

struct simple_rating_element {
  simple_rating_element(unsigned int const from, unsigned int const to)
      : from_(from), to_(to) {}
  unsigned int const from_;
  unsigned int const to_;
  std::vector<std::pair<rating_type, probability>> ratings_;
};

struct simple_connection_rating {
  std::vector<simple_rating_element> ratings_elements_;
  probability connection_rating_;
};

bool rate(simple_connection_rating&, routing::Connection const*,
          schedule const&, start_and_travel_distributions const&);

std::string to_string(rating_type const);
}  // namespace simple_rating
}  // namespace rating
}  // namespace reliability
}  // namespace motis
