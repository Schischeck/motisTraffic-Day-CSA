#include "motis/realtime/handler/connection_decision_handler.h"

#include "motis/realtime/handler/util.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/core/schedule/schedule.h"
#include "motis/protocol/RISMessage_generated.h"

void motis::realtime::handler::handle_connection_decision(
    motis::ris::ConnectionDecisionMessage const* ris_msg,
    motis::realtime::realtime_schedule& ctx) {
  auto from_event =
      ris_event_to_schedule_event(*ris_msg->from(), ctx._schedule);
  for (auto cd : *ris_msg->to()) {
    auto to_event =
        ris_event_to_schedule_event(*cd->base(), from_event, ctx._schedule);
    connection_status_decision_message msg(
        from_event, to_event,
        cd->hold() ? status_decision::kept : status_decision::unkept);
    ctx._message_handler.handle_connection_status_decision(msg);
  }
}
