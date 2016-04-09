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
  ctx.stats.assessment.inc();

  get_trip(ctx.sched, msg->from_trip_id(), ctx.stats.assessment);

  for (auto const& to : *msg->to()) {
    get_trip(ctx.sched, to->trip_id(), ctx.stats.assessment);
  }
}

}  // namespace handler
}  // namespace rt
}  // namespace motis
