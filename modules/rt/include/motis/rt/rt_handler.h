#pragma once

#include <memory>

#include "motis/core/schedule/schedule.h"

#include "motis/module/message.h"

#include "motis/rt/delay_propagator.h"
#include "motis/rt/reroute.h"
#include "motis/rt/statistics.h"

namespace motis {
namespace rt {

struct rt_handler {
  explicit rt_handler(schedule& sched);

  motis::module::msg_ptr update(motis::module::msg_ptr const&);
  motis::module::msg_ptr flush(motis::module::msg_ptr const&);

private:
  void propagate();

  schedule& sched_;
  delay_propagator propagator_;
  statistics stats_;
  std::map<schedule_event, delay_info*> cancelled_delays_;
};

}  // namespace rt
}  // namespace motis
