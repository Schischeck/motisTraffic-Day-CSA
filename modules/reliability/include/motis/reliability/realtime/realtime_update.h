#pragma once

namespace motis {
struct schedule;
namespace realtime {
struct RealtimeDelayInfoResponse;
}
namespace reliability {
struct start_and_travel_distributions;
namespace distributions_container {
struct container;
}
namespace realtime {

void update_precomputed_distributions(
    motis::realtime::RealtimeDelayInfoResponse const*, schedule const&,
    start_and_travel_distributions const&,
    distributions_container::container& precomputed_distributions);

}  // namespace realtime
}  // namespace reliability
}  // namespace motis
