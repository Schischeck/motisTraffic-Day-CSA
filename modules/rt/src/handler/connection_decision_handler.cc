#include "motis/rt/handler/connection_decision_handler.h"

#include "motis/rt/handler/context.h"
#include "motis/rt/handler/util.h"
#include "motis/protocol/RISMessage_generated.h"

using namespace motis::ris;

namespace motis {
namespace rt {
namespace handler {

void handle_connection_decision(context& ctx,
                                ConnectionDecisionMessage const* msg) {
  ctx.stats.decision.inc();

  auto trip = get_trip(ctx.sched, msg->fromTripId(), ctx.stats.decision);

  for (auto const& to : *msg->to()) {
    auto trip2 = get_trip(ctx.sched, to->tripId(), ctx.stats.decision);
  }
}

}  // namespace handler
}  // namespace rt
}  // namespace motis
