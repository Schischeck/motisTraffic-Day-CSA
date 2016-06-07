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

std::pair<std::vector<connection_rating>,
          std::vector<simple_rating::simple_connection_rating> >
rate_journeys(std::vector<journey> const& journeys, context const& c) {
  std::vector<connection_rating> ratings(journeys.size());
  std::vector<simple_rating::simple_connection_rating> simple_ratings(
      journeys.size());
  unsigned int rating_index = 0;

  for (auto const& j : journeys) {
    if (!walks_only(j)) {
      rating::rate(ratings[rating_index], j, c);
      rating::simple_rating::rate(simple_ratings[rating_index], j, c.schedule_,
                                  c.s_t_distributions_);
    }
    ++rating_index;
  }

  return std::make_pair(ratings, simple_ratings);
}

void update_mumo_and_address_infos(std::vector<journey>& journeys,
                                   bool const dep_intermodal = false,
                                   bool const arr_intermodal = false,
                                   std::string const dep_address = "",
                                   std::string const arr_address = "") {
  for (auto& j : journeys) {
    intermodal::update_mumo_info(j);
    if (dep_intermodal) {
      j.stops_.front().name_ = dep_address;
    }
    if (arr_intermodal) {
      j.stops_.back().name_ = arr_address;
    }
  }
}

module::msg_ptr rating(ReliableRoutingRequest const& req, reliability& rel,
                       unsigned const max_bikesharing_duration) {
  auto lock = rel.synced_sched();
  using routing::RoutingResponse;
  auto routing_response =
      motis_call(
          flatbuffers::request_builder(req)
              .add_additional_edges(intermodal::individual_modes_container(
                  req, max_bikesharing_duration))
              .build_routing_request())
          ->val();

  ::motis::reliability::context c(lock.sched(), *rel.precomputed_distributions_,
                                  *rel.s_t_distributions_);
  auto journeys =
      message_to_journeys(motis_content(RoutingResponse, routing_response));
  auto const ratings = rate_journeys(journeys, c);

  update_mumo_and_address_infos(
      journeys, req.dep_is_intermodal(), req.arr_is_intermodal(),
      flatbuffers::departure_station_name(*req.request()),
      req.request()->destination()->name()->str());

  return flatbuffers::response_builder::to_reliability_rating_response(
      journeys, ratings.first, ratings.second, true /* short output */);
}

}  // namespace rating
}  // namespace reliability
}  // namespace motis
