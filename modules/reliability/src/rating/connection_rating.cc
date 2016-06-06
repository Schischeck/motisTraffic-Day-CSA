#include "motis/reliability/rating/connection_rating.h"

#include "motis/core/journey/journey.h"

#include "motis/reliability/computation/distributions_calculator.h"
#include "motis/reliability/context.h"
#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/distributions/start_and_travel_distributions.h"

#include "motis/reliability/rating/connection_to_graph_data.h"
#include "motis/reliability/rating/public_transport.h"

namespace motis {
namespace reliability {
namespace rating {

void rate(connection_rating& rating, journey const& journey,
          context const& context) {
  auto const connection_elements =
      rating::connection_to_graph_data::get_elements(context.schedule_,
                                                     journey);

  public_transport::rate(rating.public_transport_ratings_, connection_elements,
                         false, context);
  if (!rating.public_transport_ratings_.empty()) {
    rating.connection_rating_ =
        rating.public_transport_ratings_.back().arrival_distribution_.sum();
  }
}

}  // namespace rating
}  // namespace reliability
}  // namespace motis
