#include "motis/reliability/rating/reliability_rating.h"

#include "motis/core/journey/journey.h"
#include "motis/core/journey/message_to_journeys.h"
#include "motis/module/context/motis_call.h"
#include "motis/protocol/RoutingResponse_generated.h"

#include "motis/reliability/context.h"
#include "motis/reliability/intermodal/individual_modes_container.h"
#include "motis/reliability/rating/connection_rating.h"
#include "motis/reliability/rating/simple_rating.h"
#include "motis/reliability/reliability.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"
#include "motis/reliability/tools/flatbuffers/response_builder.h"

namespace motis {
namespace reliability {
namespace rating {

bool walks_only(journey const& j) {
  return std::find_if(j.transports_.begin(), j.transports_.end(),
                      [](auto const& t) { return !t.is_walk_; }) ==
         j.transports_.end();
}

module::msg_ptr rate_routing_response(routing::RoutingResponse const& res,
                                      context const& c,
                                      bool const dep_intermodal,
                                      bool const arr_intermodal,
                                      std::string const dep_address,
                                      std::string const arr_address) {
  std::vector<rating::connection_rating> ratings(res.connections()->size());
  std::vector<rating::simple_rating::simple_connection_rating> simple_ratings(
      res.connections()->size());
  unsigned int rating_index = 0;
  auto const journeys = message_to_journeys(&res);

  for (auto const& j : journeys) {
    if (!walks_only(j)) {
      rating::rate(ratings[rating_index], j,
                   context(c.schedule_, c.precomputed_distributions_,
                           c.s_t_distributions_));
      rating::simple_rating::rate(simple_ratings[rating_index], j, c.schedule_,
                                  c.s_t_distributions_);
    }
    ++rating_index;
  }

  return flatbuffers::response_builder::to_reliability_rating_response(
      &res, ratings, simple_ratings, true /* short output */, dep_intermodal,
      arr_intermodal, dep_address, arr_address);
}

module::msg_ptr rating(ReliableRoutingRequest const& req, reliability& rel,
                       unsigned const max_bikesharing_duration) {
  using routing::RoutingResponse;
  auto routing_response =
      motis_call(
          flatbuffers::request_builder(req)
              .add_additional_edges(intermodal::individual_modes_container(
                  req, max_bikesharing_duration))
              .build_routing_request())
          ->val();
  auto lock = rel.synced_sched();
  return rating::rate_routing_response(
      *motis_content(RoutingResponse, routing_response),
      ::motis::reliability::context(lock.sched(),
                                    *rel.precomputed_distributions_,
                                    *rel.s_t_distributions_),
      req.dep_is_intermodal(), req.arr_is_intermodal(),
      flatbuffers::departure_station_name(*req.request()),
      req.request()->destination()->name()->str());
}

}  // namespace rating
}  // namespace reliability
}  // namespace motis
