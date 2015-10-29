#pragma once

#include "motis/module/module.h"

namespace motis {
namespace reliability {
struct reliability;
struct ReliableRoutingRequest;
namespace search {
struct connection_graph;
namespace connection_graph_search {

void search_cgs(ReliableRoutingRequest const*, reliability&, schedule const&,
                motis::module::sid, motis::module::callback);

namespace detail {
struct context;
void handle_base_response(motis::module::msg_ptr, boost::system::error_code,
                          connection_graph&, context&, motis::module::sid,
                          motis::module::callback);
}  // namespace detail
}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
