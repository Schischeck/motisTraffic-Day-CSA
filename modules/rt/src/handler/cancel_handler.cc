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
  auto trip = get_trip(ctx, msg->tripId());

}

}  // namespace handler
}  // namespace rt
}  // namespace motis
