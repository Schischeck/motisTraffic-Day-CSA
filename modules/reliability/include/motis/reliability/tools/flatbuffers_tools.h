#pragma once

#include <string>
#include <tuple>

#include "motis/module/module.h"

namespace motis {
namespace reliability {
namespace rating {
struct connection_rating;
}
namespace flatbuffers_tools {

module::msg_ptr to_flatbuffers_message(routing::RoutingRequest const* request);

module::msg_ptr to_routing_request(std::string const& from_name,
                                   std::string const& from_eva,
                                   std::string const& to_name,
                                   std::string const& to_eva,
                                   motis::time interval_begin,
                                   motis::time interval_end,
                                   std::tuple<int, int, int> ddmmyyyy);

module::msg_ptr to_reliable_routing_response(
    module::msg_ptr routing_response,
    std::vector<rating::connection_rating> const&);

}  // namespace flatbuffers_tools
}  // namespace reliability
}  // namespace motis
