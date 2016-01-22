#pragma once

#include <ostream>

namespace motis {
namespace routing {

struct statistics {
  statistics()
      : labels_created(0),
        labels_popped(0),
        labels_dominated_by_results(0),
        labels_filtered(0),
        labels_dominated_by_former_labels(0),
        labels_dominated_by_later_labels(0),
        labels_popped_until_first_result(-1),
        labels_popped_after_last_result(0),
        priority_queue_max_size(0),
        start_label_count(0),
        labels_equals_popped(0),
        max_label_quit(false) {}

  std::size_t labels_created;
  int labels_popped, labels_dominated_by_results;
  int labels_filtered, labels_dominated_by_former_labels,
      labels_dominated_by_later_labels;
  int labels_popped_until_first_result, labels_popped_after_last_result;
  int priority_queue_max_size;
  int start_label_count;
  int labels_equals_popped;
  bool max_label_quit;
  int travel_time_l_b;
  int transfers_l_b;
  int price_l_b;
  int total_calculation_time;

  friend std::ostream& operator<<(std::ostream& o, statistics const& s) {
    return o << "stats:\n"
             << "\ttravel_time_l_b: " << s.travel_time_l_b << "ms\n"
             << "\ttransfers_l_b: " << s.transfers_l_b << "ms\n"
             << "\tprice_l_b: " << s.price_l_b << "ms\n"
             << "\tstart_label_count:" << s.start_label_count << "\n"
             << "\tcreated: " << s.labels_created << "\n"
             << "\tpopped: " << s.labels_popped << "\n"
             << "\tequals_popped:" << s.labels_equals_popped << "\n"
             << "\tdominated_by_results:" << s.labels_dominated_by_results
             << "\n"
             << "\tdominated_by_former_labels:"
             << s.labels_dominated_by_former_labels << "\n"
             << "\tpopped_until_first_result:"
             << s.labels_popped_until_first_result << "\n"
             << "\tpopped_after_last_result:"
             << s.labels_popped_after_last_result << "\n"
             << "\tpriority_queue_max_size:" << s.priority_queue_max_size
             << "\n"
             << "\tfiltered:" << s.labels_filtered << "\n"
             << "\tmax_label_quit:" << std::boolalpha << s.max_label_quit
             << "\n"
             << "\ttotal_calculation_time:" << s.total_calculation_time << "\n";
  }
};

}  // namespace routing
}  // namespace motis