#pragma once

#include <vector>

namespace motis {

struct journey;

namespace routing {
struct RoutingResponse;
}  // namespace routing

std::vector<journey> message_to_journeys(routing::RoutingResponse const*);

}  // namespace motis
