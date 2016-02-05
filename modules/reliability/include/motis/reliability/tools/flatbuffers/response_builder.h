#pragma once

#include <memory>
#include <string>
#include <tuple>

#include "motis/module/module.h"

namespace motis {
struct category;
namespace reliability {
namespace rating {
struct connection_rating;
namespace simple_rating {
struct simple_connection_rating;
}
}
namespace search {
struct connection_graph;
}
namespace flatbuffers {
namespace response_builder {

module::msg_ptr to_reliability_rating_response(
    routing::RoutingResponse const*,
    std::vector<rating::connection_rating> const&,
    std::vector<rating::simple_rating::simple_connection_rating> const&,
    bool const short_output);

module::msg_ptr to_reliable_routing_response(
    std::vector<std::shared_ptr<search::connection_graph>> const&);

}  // namespace response_builder
}  // namespace flatbuffers
}  // namespace reliability
}  // namespace motis
