#include "motis/reliability/realtime/realtime_update.h"

#include "motis/protocol/RealtimeDelayInfoResponse_generated.h"

#include "motis/reliability/context.h"

namespace motis {
namespace realtime {
struct RealtimeDelayInfoResponse;
}
namespace reliability {
struct context;
namespace realtime {

void update_precomputed_distributions(
    context& c, motis::realtime::RealtimeDelayInfoResponse const* res) {

  for (auto it = res->delay_infos()->begin(); it != res->delay_infos()->end();
       ++it) {
    if (it->reason() == motis::realtime::InternalTimestampReason_Schedule) {
    }
  }
}

}  // namespace realtime
}  // namespace reliability
}  // namespace motis
