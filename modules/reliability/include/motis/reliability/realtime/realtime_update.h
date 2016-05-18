#pragma once

// TODO(Mohammad Keyhani)
#if 0
namespace motis {
struct schedule;
namespace realtime {
struct RealtimeDelayInfoResponse;  // NOLINT
}  // namespace realtime
namespace reliability {
struct start_and_travel_distributions;
namespace distributions_container {
struct container;
}  // namespace  distributions_container
namespace realtime {

void update_precomputed_distributions(
    motis::realtime::RealtimeDelayInfoResponse const*, schedule const&,
    start_and_travel_distributions const&,
    distributions_container::container& precomputed_distributions);

}  // namespace realtime
}  // namespace reliability
}  // namespace motis
#endif
