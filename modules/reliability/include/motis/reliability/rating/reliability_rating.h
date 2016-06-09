#pragma once

#include <string>
#include <utility>
#include <vector>

#include "motis/module/message.h"

#include "motis/reliability/rating/simple_rating.h"

namespace motis {
struct journey;
namespace routing {
struct RoutingResponse;  // NOLINT
}  // namespace routing
namespace reliability {
struct reliability;
struct ReliableRoutingRequest;  // NOLINT
struct context;
namespace rating {
struct connection_rating;

module::msg_ptr rating(ReliableRoutingRequest const&, reliability&,
                       unsigned const max_bikesharing_duration,
                       bool const pareto_filtering_for_bikesharing);

module::msg_ptr rate_routing_response(routing::RoutingResponse const&,
                                      motis::reliability::context const&,
                                      bool const dep_intermodal = false,
                                      bool const arr_intermodal = false,
                                      std::string const dep_address = "",
                                      std::string const arr_address = "");

std::pair<std::vector<connection_rating>,
          std::vector<simple_rating::simple_connection_rating> >
rate_journeys(std::vector<journey> const&, motis::reliability::context const&);
}  // namespace rating
}  // namespace reliability
}  // namespace motis
