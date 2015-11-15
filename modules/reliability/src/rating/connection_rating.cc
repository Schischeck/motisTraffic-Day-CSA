#include "motis/reliability/rating/connection_rating.h"

#include "motis/core/common/journey.h"

#include "motis/reliability/context.h"
#include "motis/reliability/computation/distributions_calculator.h"
#include "motis/reliability/distributions_container.h"
#include "motis/reliability/start_and_travel_distributions.h"

#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/rating/public_transport.h"

namespace motis {
namespace reliability {
namespace rating {

bool rate(connection_rating& rating, journey const& journey,
          context const& context) {
  auto const connection_elements =
      rating::connection_to_graph_data::get_elements(context.schedule_,
                                                     journey);
  if (!connection_elements.first) {
    return false;
  }
  public_transport::rate(rating.public_transport_ratings_,
                         connection_elements.second, false, context);
  rating.connection_rating_ =
      rating.public_transport_ratings_.back().arrival_distribution_.sum();

  return true;
}

}  // namespace rating
}  // namespace reliability
}  // namespace motis
