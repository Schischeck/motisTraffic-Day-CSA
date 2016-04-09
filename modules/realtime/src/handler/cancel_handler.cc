#include "motis/realtime/handler/cancel_handler.h"

#include "motis/realtime/handler/util.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/core/schedule/schedule.h"
#include "motis/protocol/RISMessage_generated.h"

void motis::realtime::handler::handle_cancel(
    motis::ris::CancelMessage const* ris_msg,
    motis::realtime::realtime_schedule& ctx) {
  if (ris_msg->events()->size() == 0) {
    return;
  }
  cancel_train_message rt_msg;
  rt_msg.events_.reserve(ris_msg->events()->size());
  for (auto ris_event : *ris_msg->events()) {
    rt_msg.events_.emplace_back(
        ris_event_to_schedule_event(*ris_event, ctx._schedule));
  }
  rt_msg.train_nr_ = ris_msg->events()->Get(0)->service_num();

  ctx._message_handler.handle_canceled_train(rt_msg);
}
