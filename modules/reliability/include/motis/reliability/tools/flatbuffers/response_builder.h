#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "motis/module/module.h"

namespace motis {
struct journey;
namespace reliability {
namespace rating {
struct connection_rating;
namespace simple_rating {
struct simple_connection_rating;
}  // namespace simple_rating
}  // namespace rating
namespace intermodal {
namespace bikesharing {
struct bikesharing_info;
}  // namespace bikesharing
}  // namespace intermodal
namespace search {
struct connection_graph;
}  // namespace search
namespace response_builder {

module::msg_ptr to_reliability_rating_response(
    std::vector<journey> const&, std::vector<rating::connection_rating> const&,
    std::vector<rating::simple_rating::simple_connection_rating> const&,
    bool const short_output,
    std::vector<std::pair<intermodal::bikesharing::bikesharing_info,
                          intermodal::bikesharing::bikesharing_info>> const&,
    bool const dep_is_intermodal, bool const arr_is_intermodal);

module::msg_ptr to_empty_reliability_rating_response();

module::msg_ptr to_reliable_routing_response(
    std::vector<std::shared_ptr<search::connection_graph>> const&);

}  // namespace response_builder
}  // namespace reliability
}  // namespace motis
