#include "motis/realtime/handler/delay_handler.h"

#include "motis/realtime/handler/util.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/core/schedule/schedule.h"
#include "motis/protocol/RISMessage_generated.h"

namespace motis {
namespace realtime {
namespace handler {

delayed_event updated_event_to_delayed_event(
    ris::UpdatedEvent const* updated_event, schedule const& sched) {
  return delayed_event(
      ris_event_to_schedule_event(*updated_event->base(), sched),
      unix_to_motistime(sched.schedule_begin_, updated_event->updatedTime()));
}

void handle_delay(motis::ris::DelayMessage const* ris_msg,
                  realtime_schedule& ctx) {
  if (ris_msg->events()->size() == 0) {
    return;
  }
  delay_message rt_msg;
  auto ris_event = ris_msg->events()->begin();
  if (ris_msg->type() == motis::ris::DelayType_Is) {
    rt_msg.is_ = updated_event_to_delayed_event(*ris_event, ctx._schedule);
    rt_msg.train_nr_ = rt_msg.is_._train_nr;
    ++ris_event;
  }
  rt_msg.forecasts_.reserve(std::distance(ris_event, ris_msg->events()->end()));
  for (; ris_event != ris_msg->events()->end(); ++ris_event) {
    rt_msg.forecasts_.emplace_back(
        updated_event_to_delayed_event(*ris_event, ctx._schedule));
  }
  if (ris_msg->type() != motis::ris::DelayType_Is) {
    rt_msg.train_nr_ = rt_msg.forecasts_[0]._train_nr;
  }

  ctx._message_handler.handle_delay(rt_msg);
}

}  // namespace handler
}  // namespace realtime
}  // namespace motis
