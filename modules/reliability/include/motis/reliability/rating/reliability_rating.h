#pragma once

#include "motis/module/message.h"

namespace motis {
namespace routing {
struct RoutingResponse;
}
namespace reliability {
struct context;
namespace rating {
module::msg_ptr rate_routing_response(routing::RoutingResponse const&,
                                      motis::reliability::context const&);
}  // namespace rating
}  // namespace reliability
}  // namespace motis
