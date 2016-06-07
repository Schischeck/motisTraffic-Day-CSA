#pragma once

namespace motis {
struct schedule;
namespace rt {
struct RtUpdate;  // NOLINT
}  // namespace realtime
namespace reliability {
struct start_and_travel_distributions;
namespace distributions_container {
struct container;
}  // namespace  distributions_container
namespace realtime {

void update_precomputed_distributions(
    motis::rt::RtUpdate const*, schedule const&,
    start_and_travel_distributions const&,
    distributions_container::container& precomputed_distributions);

}  // namespace realtime
}  // namespace reliability
}  // namespace motis
