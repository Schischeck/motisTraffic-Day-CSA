#pragma once

#include <queue>
#include <vector>

#include "motis/core/common/hash_set.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/access/event_access.h"
#include "motis/core/access/realtime_access.h"

namespace motis {
namespace rt {

struct delay_propagator {
  struct di_cmp {
    inline bool operator()(delay_info const* lhs, delay_info const* rhs) {
      return lhs->get_schedule_time() < rhs->get_schedule_time();
    }
  };

  using pq = std::priority_queue<delay_info*, std::vector<delay_info*>, di_cmp>;

  explicit delay_propagator(schedule& sched) : sched_(sched) {
    events_.set_empty_key(nullptr);
  }

  hash_set<delay_info*> const& events() const { return events_; }

  void add_delay(ev_key const& k,
                 timestamp_reason const reason = timestamp_reason::SCHEDULE,
                 time const updated_time = INVALID_TIME) {
    auto di = get_or_create_di(k);
    if (reason != timestamp_reason::SCHEDULE && di->set(reason, updated_time)) {
      expand(di->get_ev_key());
    }
  }

  void propagate() {
    while (!pq_.empty()) {
      auto di = pq_.top();
      pq_.pop();

      if (update_propagation(di)) {
        expand(di->get_ev_key());
      }
    }
  }

  void reset() {
    pq_ = pq();
    events_.clear();
  }

private:
  delay_info* get_or_create_di(ev_key const& k) {
    auto di = map_get_or_create(sched_.graph_to_delay_info_, k, [&]() {
      sched_.delay_mem_.emplace_back(std::make_unique<delay_info>(k));
      return sched_.delay_mem_.back().get();
    });
    events_.insert(di);
    return di;
  }

  void push(ev_key const& k) { pq_.push(get_or_create_di(k)); }

  void expand(ev_key const& k) {
    if (k.is_arrival()) {
      for_each_departure(k, [&](ev_key const& dep) { push(dep); });
    } else {
      push(k.get_opposite());
    }
  }

  bool update_propagation(delay_info* di) {
    auto k = di->get_ev_key();
    switch (k.ev_type_) {
      case event_type::ARR: {
        // Propagate delay from previous departure.
        auto const dep_di = get_or_create_di(k.get_opposite());
        auto const dep_sched_time = dep_di->get_schedule_time();
        auto const arr_sched_time = di->get_schedule_time();
        auto const duration = arr_sched_time - dep_sched_time;
        auto const propagated = dep_di->get_current_time() + duration;
        return di->set(timestamp_reason::PROPAGATION, propagated);
      }

      case event_type::DEP: {
        // Propagate delay from previous arrivals.
        auto max = 0;

        for_each_arrival(k, [&](ev_key const& arr) {
          auto const dep_sched_time = di->get_schedule_time();
          auto const arr_sched_time = get_schedule_time(sched_, arr);
          auto const sched_standing_time = dep_sched_time - arr_sched_time;
          auto const min_standing = std::min(2, sched_standing_time);
          auto const arr_curr_time = get_or_create_di(arr)->get_current_time();
          max = std::max(max, arr_curr_time + min_standing);
        });

        return di->set(timestamp_reason::PROPAGATION, max);
      }

      default: return false;
    }
  }

  pq pq_;
  hash_set<delay_info*> events_;
  schedule& sched_;
};

}  // namespace rt
}  // namespace motis
