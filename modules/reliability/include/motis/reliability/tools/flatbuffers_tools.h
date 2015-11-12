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
namespace flatbuffers_tools {

module::msg_ptr to_flatbuffers_message(routing::RoutingRequest const* request);

module::msg_ptr to_routing_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy, bool const ontrip);
module::msg_ptr to_routing_request(std::string const& from_name,
                                   std::string const& from_eva,
                                   std::string const& to_name,
                                   std::string const& to_eva,
                                   time_t interval_begin, time_t interval_end,
                                   bool const ontrip);

module::msg_ptr to_rating_request(std::string const& from_name,
                                  std::string const& from_eva,
                                  std::string const& to_name,
                                  std::string const& to_eva,
                                  motis::time interval_begin,
                                  motis::time interval_end,
                                  std::tuple<int, int, int> ddmmyyyy);
module::msg_ptr to_connection_tree_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy, short const num_alternatives_at_stop,
    short const min_dep_diff);
module::msg_ptr to_reliable_routing_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy, short const min_dep_diff);

module::msg_ptr to_reliability_rating_response(
    routing::RoutingResponse const*,
    std::vector<rating::connection_rating> const&,
    std::vector<rating::simple_rating::simple_connection_rating> const&,
    bool const short_output);

module::msg_ptr to_reliable_routing_response(
    std::vector<std::shared_ptr<search::connection_graph>> const&);

}  // namespace flatbuffers_tools
}  // namespace reliability
}  // namespace motis
