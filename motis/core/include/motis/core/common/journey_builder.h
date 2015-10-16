#pragma once

#include <vector>

namespace motis {
struct journey;
namespace routing {
struct RoutingResponse;
}

std::vector<journey> to_journeys(routing::RoutingResponse const*);

}  // namespace motis
