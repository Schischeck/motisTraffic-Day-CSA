#include "motis/rt/handler/cancel_handler.h"

#include <iostream>

#include "motis/rt/handler/context.h"
#include "motis/rt/handler/util.h"
#include "motis/protocol/RISMessage_generated.h"

using namespace motis::ris;

namespace motis {
namespace rt {
namespace handler {

void handle_cancel(context& ctx, CancelMessage const* msg) {
  ctx.stats.canceled.inc();
  auto trip = get_trip(ctx.sched, msg->tripId(), ctx.stats.canceled);

}

}  // namespace handler
}  // namespace rt
}  // namespace motis
