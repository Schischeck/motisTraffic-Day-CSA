#include "motis/rt/handler/connection_assessment_handler.h"

#include "motis/rt/handler/context.h"
#include "motis/rt/handler/util.h"
#include "motis/protocol/RISMessage_generated.h"

using namespace motis::ris;

namespace motis {
namespace rt {
namespace handler {

void handle_connection_assessment(context& ctx,
                                  ConnectionAssessmentMessage const* msg) {

  auto trip = get_trip(ctx, msg->fromTripId());

  for (auto const& to : *msg->to()) {
    auto trip2 = get_trip(ctx, to->tripId());
  }
}

}  // namespace handler
}  // namespace rt
}  // namespace motis
