#include "motis/reliability/rating/reliability_rating.h"

#include "motis/core/journey/journey.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/protocol/RoutingResponse_generated.h"

#include "motis/reliability/context.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/simple_rating.h"
#include "motis/reliability/tools/flatbuffers/response_builder.h"

namespace motis {
namespace reliability {
namespace rating {

module::msg_ptr rate_routing_response(routing::RoutingResponse const& res,
                                      context const& c) {
  std::vector<rating::connection_rating> ratings(res.connections()->size());
  std::vector<rating::simple_rating::simple_connection_rating> simple_ratings(
      res.connections()->size());
  unsigned int rating_index = 0;
  auto const journeys = message_to_journeys(&res);

  for (auto const& j : journeys) {
    rating::rate(ratings[rating_index], j,
                 context(c.schedule_, c.precomputed_distributions_,
                         c.s_t_distributions_));
    rating::simple_rating::rate(simple_ratings[rating_index], j, c.schedule_,
                                c.s_t_distributions_);
    ++rating_index;
  }

  return flatbuffers::response_builder::to_reliability_rating_response(
      &res, ratings, simple_ratings, true /* short output */);
}

}  // namespace rating
}  // namespace reliability
}  // namespace motis
