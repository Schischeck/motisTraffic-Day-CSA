#ifndef TDPARETO_DIJKSTRA_H
#define TDPARETO_DIJKSTRA_H TDPARETO_DIJKSTRA_H

#include <queue>
#include <list>
#include <ostream>
#include <unordered_map>

#include "motis/routing/label.h"
#include "motis/routing/lower_bounds.h"

namespace td {

class heuristic;
class node;
class station_node;

class pareto_dijkstra
{
public:
  class compare_labels
  {
    public:
      bool operator() (const label* l1, const label* l2) const
      { return l2->operator<(*l1); }
  };

  /** stores search statistics for performance analysis */
  struct statistics
  {
    statistics()
      : labels_popped(0), labels_created(0),
        labels_dominated_by_results(0), labels_filtered(0),
        labels_dominated_by_former_labels(0), labels_dominated_by_later_labels(0),
        labels_popped_until_first_result(-1),
        labels_popped_after_last_result(0),
        priority_queue_max_size(0),
        start_label_count(0),
        labels_equals_popped(0),
        max_label_quit(false)
    {}

    int labels_popped, labels_created, labels_dominated_by_results;
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
      return
      o << "stats:\n"
        << "\ttravel_time_l_b: " << s.travel_time_l_b << "ms\n"
        << "\ttransfers_l_b: " << s.transfers_l_b << "ms\n"
        << "\tprice_l_b: " << s.price_l_b << "ms\n"
        << "\tstart_label_count:" << s.start_label_count << "\n"
        << "\tcreated: " << s.labels_created << "\n"
        << "\tpopped: " << s.labels_popped << "\n"
        << "\tequals_popped:" << s.labels_equals_popped << "\n"
        << "\tdominated_by_results:" << s.labels_dominated_by_results << "\n"
        << "\tdominated_by_former_labels:" << s.labels_dominated_by_former_labels << "\n"
        << "\tpopped_until_first_result:" << s.labels_popped_until_first_result << "\n"
        << "\tpopped_after_last_result:" << s.labels_popped_after_last_result << "\n"
        << "\tpriority_queue_max_size:" << s.priority_queue_max_size << "\n"
        << "\tfiltered:" << s.labels_filtered << "\n"
        << "\tmax_label_quit:" << std::boolalpha << s.max_label_quit << "\n"
        << "\ttotal_calculation_time:" << s.total_calculation_time << "\n";
    }
  };

  pareto_dijkstra(
      int node_count,
      station_node const* goal,
      std::vector<label*> const& start_labels,
      std::unordered_map<node const*, std::vector<edge>> const& additional_edges,
      lower_bounds& lower_bounds,
      memory_manager<label>& label_store);

  std::vector<label*>& search();
  statistics get_statistics() const;

private:
  void create_new_label(label* label, edge const& edge);
  bool add_result(label* li);
  bool dominated_by_results(label* label);
  bool add_label_to_node(label* label, node const* dest);
  void filter_results();

  station_node const* _goal;
  std::vector<std::vector<label*>> _node_labels;
  std::priority_queue<label*, std::vector<label*>, compare_labels> _queue;
  std::vector<label*> _equals;
  std::unordered_map<node const*, std::vector<edge>> _additional_edges;
  std::vector<label*> _results;
  lower_bounds& _lower_bounds;
  memory_manager<label>& _label_store;
  statistics _stats;
};

}  // namespace td

#endif //TDPARETO_DIJKSKTRA_H

