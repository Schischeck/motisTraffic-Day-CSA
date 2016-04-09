#include "motis/rt/handler/reroute_handler.h"

#include "motis/rt/handler/context.h"
#include "motis/rt/handler/util.h"
#include "motis/protocol/RISMessage_generated.h"

using namespace motis::ris;

namespace motis {
namespace rt {
namespace handler {

void handle_reroute(context& ctx, RerouteMessage const* msg) {
  ctx.stats.reroute.inc();
  get_trip(ctx.sched, msg->trip_id(), ctx.stats.reroute);
}

}  // namespace handler
}  // namespace rt
}  // namespace motis
