#include "motis/reliability/rating/connection_rating.h"

#include "motis/core/common/journey.h"

#include "motis/reliability/computation/distributions_calculator.h"
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/start_and_travel_distributions.h"

#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/rating/public_transport.h"

namespace motis {
namespace reliability {
namespace rating {

bool rate(connection_rating& rating, journey const& journey,
          schedule const& schedule,
          distributions_container::precomputed_distributions_container const&
              precomputed_distributions,
          start_and_travel_distributions const& s_t_distributions) {
  auto const connection_elements =
      rating::connection_to_graph_data::get_elements(schedule, journey);
  if (!connection_elements.first) {
    return false;
  }
  public_transport::rate(rating.public_transport_ratings_,
                         connection_elements.second, false, schedule,
                         precomputed_distributions, s_t_distributions);
  rating.connection_rating_ =
      rating.public_transport_ratings_.back().arrival_distribution_.sum();

  return true;
}

}  // namespace rating
}  // namespace reliability
}  // namespace motis
