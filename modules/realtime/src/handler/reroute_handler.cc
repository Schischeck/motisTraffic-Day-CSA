#include "motis/realtime/handler/reroute_handler.h"

#include "motis/realtime/handler/util.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/core/schedule/schedule.h"
#include "motis/protocol/RISMessage_generated.h"

void motis::realtime::handler::handle_reroute(
    motis::ris::RerouteMessage const* ris_msg,
    motis::realtime::realtime_schedule& ctx) {
  reroute_train_message rt_msg;
  rt_msg.canceled_events_.reserve(ris_msg->cancelled_events()->size());
  rt_msg.new_events_.reserve(ris_msg->new_events()->size());

  for (auto canceled_event : *ris_msg->cancelled_events()) {
    rt_msg.canceled_events_.emplace_back(
        ris_event_to_schedule_event(*canceled_event, ctx._schedule));
  }
  for (auto new_event : *ris_msg->new_events()) {
    rt_msg.new_events_.emplace_back(
        ris_event_to_schedule_event(*new_event->base()->base(), ctx._schedule));
  }

  if (ris_msg->new_events()->size() > 0) {
    rt_msg.category_ =
        ris_msg->new_events()->Get(0)->base()->category()->str();
    rt_msg.train_nr_ = rt_msg.new_events_[0]._train_nr;
  } else if (rt_msg.canceled_events_.size() > 0) {
    rt_msg.train_nr_ = rt_msg.canceled_events_[0]._train_nr;
  }

  ctx._message_handler.handle_rerouted_train(rt_msg);
}
