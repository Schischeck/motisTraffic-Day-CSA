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
          std::vector<simple_rating::simple_connection_rating>>
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

void update_mumo_and_address_infos(
    std::vector<journey>& journeys,
    intermodal::individual_modes_container const& container,
    bool const dep_intermodal = false, bool const arr_intermodal = false,
    std::string const dep_address = "", std::string const arr_address = "") {
  for (auto& j : journeys) {
    intermodal::update_mumo_info(j, container);
    if (dep_intermodal) {
      j.stops_.front().name_ = dep_address;
    }
    if (arr_intermodal) {
      j.stops_.back().name_ = arr_address;
    }
  }
}

using bs_type = intermodal::bikesharing::bikesharing_info;
using bs_return_type = std::vector<std::pair<bs_type, bs_type>>;
bs_return_type get_bikesharings(
    std::vector<journey>& journeys,
    intermodal::individual_modes_container const& container,
    bool const dep_is_intermodal, bool const arr_is_intermodal) {
  auto check = [&container](bool const is_intermodal, int const mumo_id) {
    return is_intermodal &&
           container.get_mumo_type(mumo_id) == intermodal::BIKESHARING;
  };
  bs_return_type bikesharings;
  if (dep_is_intermodal || arr_is_intermodal) {
    for (auto& j : journeys) {
      bikesharings.emplace_back();
      if (check(dep_is_intermodal, j.transports_.front().mumo_id_)) {
        bikesharings.back().first = intermodal::get_bikesharing_info(
            container, j.transports_.front().mumo_id_);
      }
      if (check(arr_is_intermodal, j.transports_.back().mumo_id_)) {
        bikesharings.back().second = intermodal::get_bikesharing_info(
            container, j.transports_.back().mumo_id_);
      }
    }
  }
  return bikesharings;
}

module::msg_ptr rating(ReliableRoutingRequest const& req, reliability& rel,
                       unsigned const max_bikesharing_duration,
                       bool const pareto_filtering_for_bikesharing) {
  auto lock = rel.synced_sched();
  intermodal::individual_modes_container container(
      req, max_bikesharing_duration, pareto_filtering_for_bikesharing);
  using routing::RoutingResponse;
  auto routing_response = motis_call(request_builder(req)
                                         .add_additional_edges(container)
                                         .build_routing_request())
                              ->val();

  ::motis::reliability::context c(lock.sched(), *rel.precomputed_distributions_,
                                  *rel.s_t_distributions_);
  auto journeys =
      message_to_journeys(motis_content(RoutingResponse, routing_response));
  auto const ratings = rate_journeys(journeys, c);

  update_mumo_and_address_infos(journeys, container, req.dep_is_intermodal(),
                                req.arr_is_intermodal(),
                                departure_station_name(*req.request()),
                                req.request()->destination()->name()->str());

  auto const bikesharings = get_bikesharings(
      journeys, container, req.dep_is_intermodal(), req.arr_is_intermodal());

  return response_builder::to_reliability_rating_response(
      journeys, ratings.first, ratings.second, true /* short output */,
      bikesharings, req.dep_is_intermodal(), req.arr_is_intermodal());
}

}  // namespace rating
}  // namespace reliability
}  // namespace motis
