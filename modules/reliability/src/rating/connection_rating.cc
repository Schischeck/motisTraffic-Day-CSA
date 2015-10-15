#include "motis/reliability/rating/connection_rating.h"

#include "motis/protocol/RoutingResponse_generated.h"

#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/start_and_travel_distributions.h"

#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/rating/public_transport.h"

namespace motis {
namespace reliability {
namespace rating {

std::vector<rating_element> rate(
    routing::Connection const* connection, schedule const& schedule,
    distributions_container::precomputed_distributions_container const&
        precomputed_distributions,
    start_and_travel_distributions const& s_t_distributions) {
  auto const elements =
      rating::connection_to_graph_data::get_elements(schedule, connection);

  std::vector<rating_element> ratings;
  public_transport::rate(ratings, elements, schedule, precomputed_distributions,
                         s_t_distributions);

  return ratings;
}

}  // namespace rating
}  // namespace reliability
}  // namespace motis
