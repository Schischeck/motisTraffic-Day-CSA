#pragma once

#include <string>

#include "motis/module/message.h"

namespace motis {
namespace routing {
struct RoutingResponse;  // NOLINT
}  // namespace routing
namespace reliability {
struct context;
namespace rating {
module::msg_ptr rate_routing_response(routing::RoutingResponse const&,
                                      motis::reliability::context const&,
                                      bool const dep_intermodal = false,
                                      bool const arr_intermodal = false,
                                      std::string const dep_address = "",
                                      std::string const arr_address = "");
}  // namespace rating
}  // namespace reliability
}  // namespace motis
