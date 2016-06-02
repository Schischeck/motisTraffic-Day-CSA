#pragma once

#include <queue>
#include <vector>

#include "motis/core/common/hash_set.h"
#include "motis/core/schedule/schedule.h"

namespace motis {
namespace rt {

struct delay_propagator {
  delay_propagator(schedule& sched) : sched_(sched) {
    updates_.set_empty_key(nullptr);
  }

  void add_delay(graph_event const& k, delay_info::reason const reason,
                 time const schedule_time, time const updated_time) {
    auto di = map_get_or_create(sched_.graph_to_delay_info_, k, [&]() {
      sched_.delay_mem_.emplace_back(new delay_info(k, schedule_time));
      return sched_.delay_mem_.back().get();
    });
    di->set(reason, updated_time);
    updates_.insert(di);
  }

  hash_set<delay_info*> updates_;
  schedule& sched_;
};

}  // namespace rt
}  // namespace motis
