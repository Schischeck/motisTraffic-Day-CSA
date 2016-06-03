#pragma once

#include <queue>
#include <vector>

#include "motis/core/common/hash_set.h"
#include "motis/core/schedule/schedule.h"

namespace motis {
namespace rt {

struct delay_propagator {
  explicit delay_propagator(schedule const& sched) : sched_(sched) {
    events_.set_empty_key({nullptr, 0, event_type::DEP});
  }

  void add_delay(graph_event const& k, delay_info::reason const reason,
                 time const schedule_time, time const updated_time) {
    map_get_or_create(events_, k, [&]() {
      std::unique_ptr<delay_info> di;

      auto it = sched_.graph_to_delay_info_.find(k);
      if (it == end(sched_.graph_to_delay_info_)) {
        di = std::make_unique<delay_info>(k, schedule_time);
      } else {
        di = std::make_unique<delay_info>(*it->second);
      }

      delay_mem_.emplace_back(std::move(di));

      return delay_mem_.back().get();
    })->set(reason, updated_time);
  }

  std::vector<std::unique_ptr<delay_info>> delay_mem_;
  hash_map<graph_event, delay_info*> events_;
  schedule const& sched_;
};

}  // namespace rt
}  // namespace motis
