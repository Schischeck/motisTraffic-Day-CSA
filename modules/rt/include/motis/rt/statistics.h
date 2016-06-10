#pragma once

#include <iomanip>
#include <ostream>

namespace motis {
namespace rt {

struct statistics {
  statistics()
      : total_evs_(0),
        ev_invalid_time_(0),
        ev_station_not_found_(0),
        ev_trp_not_found_(0),
        ev_exact_trp_not_found_(0),
        additional_not_found_(0),
        total_updates_(0),
        found_updates_(0),
        update_mismatch_sched_time_(0),
        diff_gt_5_(0),
        diff_gt_10_(0),
        diff_gt_30_(0),
        disabled_routes_(0),
        route_overtake_(0),
        propagated_updates_(0),
        graph_updates_(0) {}

  friend std::ostream& operator<<(std::ostream& o, statistics const& s) {
    auto c = [&](char const* desc, unsigned number) {
      o << "  " << std::setw(22) << desc << ": " << std::setw(9) << number
        << "\n";
    };

    o << "\nevs:\n";
    c("total", s.total_evs_);
    c("invalid time", s.ev_invalid_time_);
    c("station not found", s.ev_station_not_found_);
    c("trip not found", s.ev_trp_not_found_);
    c("exact trip not found", s.ev_exact_trp_not_found_);
    c("additional train event", s.additional_not_found_);

    o << "\nupdates\n";
    c("total", s.total_updates_);
    c("found", s.found_updates_);
    c("sched time mismatch", s.update_mismatch_sched_time_);
    c("time diff >5min", s.diff_gt_5_);
    c("time diff >10min", s.diff_gt_10_);
    c("time diff >30min", s.diff_gt_30_);

    o << "\ndisabled routes\n";
    c("total", s.disabled_routes_);
    c("overtake", s.route_overtake_);

    o << "\ngraph\n";
    c("propagated", s.propagated_updates_);
    c("checked", s.graph_updates_);
    c("skipped", s.propagated_updates_ - s.graph_updates_);

    return o;
  }

  unsigned total_evs_;
  unsigned ev_invalid_time_;
  unsigned ev_station_not_found_;
  unsigned ev_trp_not_found_;
  unsigned ev_exact_trp_not_found_;
  unsigned additional_not_found_;

  unsigned total_updates_;
  unsigned found_updates_;
  unsigned update_mismatch_sched_time_;
  unsigned diff_gt_5_, diff_gt_10_, diff_gt_30_;

  unsigned disabled_routes_;
  unsigned route_overtake_;

  unsigned propagated_updates_;
  unsigned graph_updates_;
};

}  // namespace rt
}  // namespace motis
