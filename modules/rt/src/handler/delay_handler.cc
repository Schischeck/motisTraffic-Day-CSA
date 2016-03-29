#include "motis/rt/handler/delay_handler.h"

#include "motis/rt/handler/context.h"
#include "motis/rt/handler/util.h"
#include "motis/protocol/RISMessage_generated.h"

using namespace motis::ris;

namespace motis {
namespace rt {
namespace handler {

void handle_delay(context& ctx, DelayMessage const* msg) {
  ctx.stats.delay.inc();
  auto trip = get_trip(ctx.sched, msg->tripId(), ctx.stats.delay);
}

}  // namespace handler
}  // namespace rt
}  // namespace motis
