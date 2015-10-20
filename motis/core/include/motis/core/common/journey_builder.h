#pragma once

#include <memory>
#include <vector>

namespace motis {
struct category;
struct journey;
namespace routing {
struct RoutingResponse;
}

std::vector<journey> to_journeys(routing::RoutingResponse const*,
                                 std::vector<std::unique_ptr<category>> const&);

}  // namespace motis
