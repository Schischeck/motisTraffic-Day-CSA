#include <algorithm>
#include <limits>
#include <vector>

#include "motis/core/schedule/delay_info.h"
#include "motis/rt/bfs.h"

namespace motis {
namespace rt {

struct entry;

constexpr auto tmax = std::numeric_limits<motis::time>::max();
constexpr auto tmin = std::numeric_limits<motis::time>::min();

struct entry : public delay_info {
  entry(ev_key const& k) : delay_info(k), min_(tmin), max_(tmax) {}
  entry(delay_info const& di) : delay_info(di), min_(tmin), max_(tmax) {}

  void fix() {
    if (get_current_time() > max_) {
      set(timestamp_reason::REPAIR, max_);
    }
    if (get_current_time() < min_) {
      set(timestamp_reason::REPAIR, min_);
    }
  }

  motis::time min_, max_;
};

struct trip_corrector {
  explicit trip_corrector(schedule const& sched) : sched_(sched) {}

  void apply_is(ev_key const& k, time is) {
    auto const future_events = trip_bfs(k, bfs_direction::FORWARD);
    for (auto const& k : future_events) {
      // set min
    }

    auto const past_events = trip_bfs(k, bfs_direction::BACKWARD);
    for (auto const& k : past_events) {
      // set max
    }
  }

  void set_min(std::set<edge const*> const& edges,
               std::map<ev_key, entry>& entries) {
    for (auto const& e : edges) {
    }
  }

  void fix_times(ev_key const& k, schedule const& sched) {
    std::map<ev_key, entry> entries;

    auto const trip_events = trip_bfs(k, bfs_direction::BOTH);
    for (auto const& k : trip_events) {
      auto di = get_delay_info(sched, k);
      if (di == nullptr || di->get_is_time() == 0) {
        continue;
      }

      apply_is(k, di->get_is_time());
    }
  }

  schedule const& sched_;
  std::map<ev_key, entry> entries_;
};

}  // namespace rt
}  // namespace motis
