#include "motis/rt/handler/delay_handler.h"

#include "motis/rt/handler/context.h"
#include "motis/rt/handler/util.h"
#include "motis/protocol/RISMessage_generated.h"

using namespace motis::ris;

namespace motis {
namespace rt {
namespace handler {

void handle_delay(context& ctx, DelayMessage const* msg) {
  auto trip = get_trip(ctx, msg->tripId());
}

}  // namespace handler
}  // namespace rt
}  // namespace motis
