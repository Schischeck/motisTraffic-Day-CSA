#include <algorithm>
#include <limits>
#include <vector>

#include "motis/core/common/get_or_create.h"
#include "motis/core/schedule/schedule.h"
#include "motis/rt/bfs.h"

namespace motis {

namespace rt {

struct entry;

constexpr auto tmax = std::numeric_limits<motis::time>::max();
constexpr auto tmin = std::numeric_limits<motis::time>::min();

struct entry : public delay_info {
  entry() = default;
  explicit entry(ev_key const& k) : delay_info(k), min_(tmin), max_(tmax) {}
  explicit entry(delay_info const& di)
      : delay_info(di), min_(tmin), max_(tmax) {}

  void update_min(time t) { min_ = std::max(min_, t); }
  void update_max(time t) { max_ = std::min(max_, t); }

  void fix() {
    if (get_current_time() > max_) {
      set(timestamp_reason::REPAIR, max_);
    }
    if (get_current_time() < min_) {
      set(timestamp_reason::REPAIR, min_);
    }
    if (get_current_time() == get_original_time()) {
      set(timestamp_reason::REPAIR, 0);
    }
  }

  motis::time min_, max_;
};

struct trip_corrector {
  explicit trip_corrector(schedule& sched, ev_key const& k)
      : sched_(sched), trip_ev_keys_(trip_bfs(k, bfs_direction::BOTH)) {}

  std::vector<delay_info*> fix_times() {
    set_min_max();
    repair();
    return update();
  }

private:
  entry& get_or_create(ev_key const& k) {
    return motis::get_or_create(entries_, k, [&]() {
      auto di_it = sched_.graph_to_delay_info_.find(k);
      if (di_it == end(sched_.graph_to_delay_info_)) {
        return entry{delay_info{k}};
      } else {
        return entry{*di_it->second};
      }
    });
  }

  void apply_is(ev_key const& k, time is) {
    for (auto const& fwd : trip_bfs(k, bfs_direction::FORWARD)) {
      get_or_create(fwd).update_min(is);
    }
    for (auto const& bwd : trip_bfs(k, bfs_direction::BACKWARD)) {
      get_or_create(bwd).update_max(is);
    }
  }

  void set_min_max() {
    for (auto const& k : trip_ev_keys_) {
      auto di = get_delay_info(sched_, k);
      if (di == nullptr || di->get_is_time() == 0) {
        continue;
      }
      apply_is(k, di->get_is_time());
    }
  }

  void repair() {
    for (auto const& k : trip_ev_keys_) {
      entries_[k].fix();
    }
  }

  std::vector<delay_info*> update() {
    std::vector<delay_info*> updates;
    for (auto const& k : trip_ev_keys_) {
      auto& e = entries_[k];
      if (e.get_reason() == timestamp_reason::REPAIR &&
          e.get_repair_time() != k.get_time()) {
        auto& di = sched_.graph_to_delay_info_[k];
        di->set(timestamp_reason::REPAIR, e.get_repair_time());

        auto& event_time = k.ev_type_ == event_type::DEP ? k.lcon()->d_time_
                                                         : k.lcon()->a_time_;
        const_cast<time&>(event_time) = di->get_current_time();  // NOLINT
      }
    }
    return updates;
  }

  schedule& sched_;
  std::set<ev_key> trip_ev_keys_;
  std::map<ev_key, entry> entries_;
};

}  // namespace rt
}  // namespace motis
