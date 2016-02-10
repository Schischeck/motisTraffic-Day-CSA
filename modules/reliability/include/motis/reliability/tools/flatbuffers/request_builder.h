#pragma once

#include <string>

#include "motis/module/module.h"

#include "motis/core/common/date_util.h"

namespace motis {
namespace reliability {
namespace intermodal {
namespace hotels {
struct hotel_info;
}
}
namespace flatbuffers {
namespace request_builder {
module::msg_ptr to_flatbuffers_message(routing::RoutingRequest const*);

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
module::msg_ptr to_reliable_routing_request(std::string const& from_name,
                                            std::string const& from_eva,
                                            std::string const& to_name,
                                            std::string const& to_eva,
                                            std::time_t interval_begin,
                                            std::time_t interval_end,
                                            short const min_dep_diff);

module::msg_ptr to_reliable_late_connections_request(
    std::string const& from_name, std::string const& from_eva,
    std::string const& to_name, std::string const& to_eva,
    motis::time interval_begin, motis::time interval_end,
    std::tuple<int, int, int> ddmmyyyy,
    /* taxi-info: from-station, duration, price */
    std::vector<std::tuple<std::string, unsigned short, unsigned short>> const&
        taxi_infos);

module::msg_ptr to_routing_late_connections_message(
    routing::RoutingRequest const*,
    std::vector<intermodal::hotels::hotel_info> const& hotel_infos);

}  // namespace request_builder
}  // namespace flatbuffers
}  // namespace reliability
}  // namespace motis
