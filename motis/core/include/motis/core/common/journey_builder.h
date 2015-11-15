#pragma once

#include <memory>
#include <vector>

namespace motis {
struct category;
struct journey;
namespace routing {
struct RoutingResponse;
}

namespace journey_builder {
std::vector<journey> to_journeys(routing::RoutingResponse const*);

namespace detail {
uint16_t get_duration(journey const& journey);
uint16_t get_transfers(journey const& journey);
}  // namespace detail
}  // namespace journey_builder
}  // namespace motis
