#pragma once

#include <ostream>

#include "motis/protocol/RoutingResponse_generated.h"

namespace motis {
namespace routing {

struct statistics {
  statistics()
      : labels_created_(0),
        labels_popped_(0),
        labels_dominated_by_results_(0),
        labels_filtered_(0),
        labels_dominated_by_former_labels_(0),
        labels_dominated_by_later_labels_(0),
        labels_popped_until_first_result_(-1),
        labels_popped_after_last_result_(0),
        priority_queue_max_size_(0),
        start_label_count_(0),
        labels_equals_popped_(0),
        max_label_quit_(false),
        travel_time_lb_(0),
        transfers_lb_(0),
        price_l_b_(0),
        total_calculation_time_(0),
        pareto_dijkstra_(0),
        num_bytes_in_use_(0) {}

  explicit statistics(int travel_time_lb) : statistics() {
    travel_time_lb_ = travel_time_lb;
  }

  std::size_t labels_created_;
  int labels_popped_, labels_dominated_by_results_;
  int labels_filtered_, labels_dominated_by_former_labels_,
      labels_dominated_by_later_labels_;
  int labels_popped_until_first_result_, labels_popped_after_last_result_;
  int priority_queue_max_size_;
  int start_label_count_;
  int labels_equals_popped_;
  bool max_label_quit_;
  int travel_time_lb_;
  int transfers_lb_;
  int price_l_b_;
  int total_calculation_time_;
  int pareto_dijkstra_;
  int num_bytes_in_use_;

  friend std::ostream& operator<<(std::ostream& o, statistics const& s) {
    return o << "stats:\n"
             << "\ttravel_time_lb: " << s.travel_time_lb_ << "ms\n"
             << "\ttransfers_lb: " << s.transfers_lb_ << "ms\n"
             << "\tprice_l_b: " << s.price_l_b_ << "ms\n"
             << "\tstart_label_count:" << s.start_label_count_ << "\n"
             << "\tcreated: " << s.labels_created_ << "\n"
             << "\tpopped: " << s.labels_popped_ << "\n"
             << "\tequals_popped:" << s.labels_equals_popped_ << "\n"
             << "\tdominated_by_results:" << s.labels_dominated_by_results_
             << "\n"
             << "\tdominated_by_former_labels:"
             << s.labels_dominated_by_former_labels_ << "\n"
             << "\tpopped_until_first_result:"
             << s.labels_popped_until_first_result_ << "\n"
             << "\tpopped_after_last_result:"
             << s.labels_popped_after_last_result_ << "\n"
             << "\tpriority_queue_max_size:" << s.priority_queue_max_size_
             << "\n"
             << "\tfiltered:" << s.labels_filtered_ << "\n"
             << "\tmax_label_quit:" << std::boolalpha << s.max_label_quit_
             << "\n"
             << "\ttotal_calculation_time:" << s.total_calculation_time_
             << "\tnum_bytes_in_use:" << s.num_bytes_in_use_ << "\n";
  }

  friend Statistics to_fbs(statistics const& s) {
    return Statistics(
        s.max_label_quit_, s.labels_created_, s.start_label_count_,
        s.labels_popped_, s.labels_equals_popped_, s.labels_filtered_,
        s.labels_dominated_by_results_, s.labels_dominated_by_former_labels_,
        s.labels_dominated_by_later_labels_,
        s.labels_popped_until_first_result_, s.labels_popped_after_last_result_,
        s.priority_queue_max_size_, s.travel_time_lb_, s.transfers_lb_,
        s.total_calculation_time_, s.pareto_dijkstra_, s.num_bytes_in_use_);
  }
};

}  // namespace routing
}  // namespace motis
