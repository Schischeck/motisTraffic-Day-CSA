#pragma once

#include <iomanip>
#include <ostream>

#include "motis/protocol/RISMessage_generated.h"

namespace motis {
namespace rt {

struct statistics {
  statistics()
      : delay_msgs_(0),
        cancel_msgs_(0),
        additional_msgs_(0),
        reroute_msgs_(0),
        con_decision_msgs_(0),
        con_assessment_msgs_(0),
        total_evs_(0),
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
        conflicting_events_(0),
        conflicting_moved_(0),
        route_overtake_(0),
        propagated_updates_(0),
        graph_updates_(0) {}

  friend std::ostream& operator<<(std::ostream& o, statistics const& s) {
    auto c = [&](char const* desc, unsigned number) {
      o << "  " << std::setw(22) << desc << ": " << std::setw(9) << number
        << "\n";
    };

    o << "\nmsg types:\n";
    c("delay", s.delay_msgs_);
    c("cancel", s.cancel_msgs_);
    c("additional", s.additional_msgs_);
    c("reroute", s.reroute_msgs_);
    c("conn decision", s.con_decision_msgs_);
    c("conn assessment", s.con_assessment_msgs_);

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
    c("conflicts", s.conflicting_events_);
    c("moved", s.conflicting_moved_);
    c("overtake", s.route_overtake_);

    o << "\ngraph\n";
    c("propagated", s.propagated_updates_);
    c("checked", s.graph_updates_);
    c("skipped", s.propagated_updates_ - s.graph_updates_);

    return o;
  }

  void log_sched_time_mismatch(int diff) {
    if (diff != 0) {
      ++update_mismatch_sched_time_;
    }
    if (diff > 5) {
      ++diff_gt_5_;
      if (diff > 10) {
        ++diff_gt_10_;
        if (diff > 30) {
          ++diff_gt_30_;
        }
      }
    }
  }

  void count_message(motis::ris::MessageUnion const type) {
    switch (type) {
      case motis::ris::MessageUnion_DelayMessage: ++delay_msgs_; break;
      case motis::ris::MessageUnion_CancelMessage: ++cancel_msgs_; break;
      case motis::ris::MessageUnion_AdditionMessage: ++additional_msgs_; break;
      case motis::ris::MessageUnion_RerouteMessage: ++reroute_msgs_; break;
      case motis::ris::MessageUnion_ConnectionDecisionMessage:
        ++con_decision_msgs_;
        break;
      case motis::ris::MessageUnion_ConnectionAssessmentMessage:
        ++con_assessment_msgs_;
        break;
      default: break;
    }
  }

  unsigned delay_msgs_;
  unsigned cancel_msgs_;
  unsigned additional_msgs_;
  unsigned reroute_msgs_;
  unsigned con_decision_msgs_;
  unsigned con_assessment_msgs_;

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

  unsigned conflicting_events_;
  unsigned conflicting_moved_;
  unsigned route_overtake_;

  unsigned propagated_updates_;
  unsigned graph_updates_;
};

}  // namespace rt
}  // namespace motis
