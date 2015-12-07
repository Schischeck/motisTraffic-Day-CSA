#pragma once

namespace motis {
namespace realtime {
struct RealtimeDelayInfoResponse;
}
namespace reliability {
struct context;
namespace realtime {

void update_precomputed_distributions(
    context&, motis::realtime::RealtimeDelayInfoResponse const*);

}  // namespace realtime
}  // namespace reliability
}  // namespace motis
