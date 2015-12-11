#include "motis/realtime/handler/delay_handler.h"

#include "motis/protocol/RISMessage_generated.h"

#include "motis/core/common/logging.h"
#include "motis/core/schedule/schedule.h"
#include "motis/realtime/handler/util.h"
#include "motis/realtime/realtime_schedule.h"

using namespace motis::ris;
using namespace motis::logging;

namespace motis {
namespace realtime {
namespace handler {

void handle_delay(DelayMessage const* ris_msg, realtime_schedule& ctx) {
  if (ris_msg->events()->size() == 0) {
    return;
  }

  ctx._stats._counters.delay_msgs.increment();
  auto const& sched = ctx._schedule;

  bool ignored = true;
  for (auto const& ris_evt : *ris_msg->events()) {
    auto schedule_evt = ris_event_to_schedule_event(*ris_evt->base(), sched);
    auto reason = ris_msg->type() == DelayType_Is ? timestamp_reason::IS
                                                  : timestamp_reason::FORECAST;
    auto updated_time =
        unix_to_motistime(sched.schedule_begin_, ris_evt->updatedTime());

    if (schedule_evt.valid() && updated_time != motis::INVALID_TIME) {
      handle_delay(ctx, schedule_evt, updated_time, reason);
      ignored = false;
    }
  }

  if (ignored) {
    ctx._stats._counters.delay_msgs.ignore();
  }
}

void handle_delay(realtime_schedule& ctx, schedule_event const& evt,
                  motis::time updated_time, timestamp_reason reason) {
  bool old_debug = ctx._debug_mode;
  if (ctx.is_tracked(evt._train_nr)) {
    // ctx._tracking.in_message(msg); // XXX
    ctx._debug_mode = true;
  }

  ctx._stats._counters.delay_events.increment();
  if (reason == timestamp_reason::IS) {
    ctx._stats._counters.delay_is.increment();
  } else if (reason == timestamp_reason::FORECAST) {
    ctx._stats._counters.delay_fc.increment();
  }

  if (std::abs(updated_time - evt._schedule_time) > MINUTES_A_DAY) {
    LOG(warn) << "invalid delay message received for " << evt
              << ", updated_time=" << motis::format_time(updated_time)
              << ", reason=" << reason << " - ignoring message";

    ctx._stats._counters.delay_events.ignore();
    if (reason == timestamp_reason::IS) {
      ctx._stats._counters.delay_is.ignore();
    } else if (reason == timestamp_reason::FORECAST) {
      ctx._stats._counters.delay_fc.ignore();
    }
    return;
  }

  ctx._delay_propagator.handle_delay_message(evt, updated_time, reason);
  ctx._debug_mode = old_debug;
}

}  // namespace handler
}  // namespace realtime
}  // namespace motis
