#pragma once

#include <list>
#include <ostream>
#include <queue>
#include <unordered_map>

#include "motis/routing/memory_manager.h"
#include "motis/routing/statistics.h"

namespace motis {
namespace routing {

const bool FORWARDING = true;

template <typename Label, typename LowerBounds>
class pareto_dijkstra {
public:
  class compare_labels {
  public:
    bool operator()(const Label* l1, const Label* l2) const {
      return l2->operator<(*l1);
    }
  };

  pareto_dijkstra(
      int node_count, station_node const* goal,
      std::vector<Label*> const& start_labels,
      std::unordered_map<node const*, std::vector<edge>> additional_edges,
      LowerBounds& lower_bounds, memory_manager& label_store)
      : goal_(goal),
        node_labels_(node_count),
        queue_(begin(start_labels), end(start_labels)),
        additional_edges_(std::move(additional_edges)),
        lower_bounds_(lower_bounds),
        label_store_(label_store),
        max_labels_(label_store.size() / sizeof(Label)) {
    for (auto const& start_label : start_labels) {
      node_labels_[start_label->node_->id_].emplace_back(start_label);
    }
  }

  std::vector<Label*>& search() {
    stats_.start_label_count_ = queue_.size();
    stats_.labels_created_ = label_store_.used_size() / sizeof(Label);

    while (!queue_.empty() || !equals_.empty()) {
      if ((stats_.labels_created_ > (max_labels_ / 2) && results_.empty()) ||
          stats_.labels_created_ > max_labels_) {
        stats_.max_label_quit_ = true;
        printf("max label quit\n");
        filter_results();
        return results_;
      }

      // get best label
      Label* label;
      if (!equals_.empty()) {
        label = equals_.back();
        equals_.pop_back();
        stats_.labels_equals_popped_++;
      } else {
        label = queue_.top();
        stats_.priority_queue_max_size_ = std::max(
            stats_.priority_queue_max_size_, static_cast<int>(queue_.size()));
        queue_.pop();
        stats_.labels_popped_++;
        stats_.labels_popped_after_last_result_++;
      }

      // is label already made obsolete
      if (label->dominated_) {
        stats_.labels_dominated_by_later_labels_++;
        continue;
      }

      if (dominated_by_results(label)) {
        stats_.labels_dominated_by_results_++;
        continue;
      }

      if (label->node_ == goal_) {
        continue;
      }

      auto it = additional_edges_.find(label->node_);
      if (it != std::end(additional_edges_)) {
        for (auto const& additional_edge : it->second) {
          create_new_label(label, additional_edge);
        }
      }

      for (auto const& edge : label->node_->edges_) {
        create_new_label(label, edge);
      }
    }

    filter_results();
    return results_;
  }

  statistics get_statistics() const { return stats_; };

private:
  void create_new_label(Label* l, edge const& edge) {
    Label blank;
    bool created = l->create_label(blank, edge, lower_bounds_);
    if (!created) {
      return;
    }

    auto new_label = new (label_store_.create<Label>()) Label(blank);
    ++stats_.labels_created_;

    if (edge.get_destination() == goal_) {
      printf("adding result\n");
      add_result(new_label);
      if (stats_.labels_popped_until_first_result_ == -1) {
        stats_.labels_popped_until_first_result_ = stats_.labels_popped_;
      }
      return;
    }

    // if the label is not dominated by a former one for the same node...
    //...add it to the queue
    if (!dominated_by_results(new_label)) {
      if (add_label_to_node(new_label, edge.get_destination())) {
        // if the new_label is as good as label we don't have to push it into
        // the queue
        if (!FORWARDING || l < new_label) {
          queue_.push(new_label);
        } else {
          equals_.push_back(new_label);
        }
      } else {
        stats_.labels_dominated_by_former_labels_++;
      }
    } else {
      stats_.labels_dominated_by_results_++;
    }
  }

  bool add_result(Label* terminal_label) {
    for (auto it = results_.begin(); it != results_.end();) {
      Label* o = *it;
      if (terminal_label->dominates(*o)) {
        it = results_.erase(it);
      } else if (o->dominates(*terminal_label)) {
        return false;
      } else {
        ++it;
      }
    }
    results_.push_back(terminal_label);
    stats_.labels_popped_after_last_result_ = 0;
    return true;
  }

  bool add_label_to_node(Label* new_label, node const* dest) {
    auto& dest_labels = node_labels_[dest->id_];
    for (auto it = dest_labels.begin(); it != dest_labels.end();) {
      Label* o = *it;
      if (o->dominates(*new_label)) {
        return false;
      }

      if (new_label->dominates(*o)) {
        it = dest_labels.erase(it);
        o->dominated_ = true;
      } else {
        ++it;
      }
    }

    // it is very important for the performance to push front here
    // because earlier labels tend not to dominate later ones (not comparable)
    dest_labels.insert(std::begin(dest_labels), new_label);
    return true;
  }

  bool dominated_by_results(Label* label) {
    for (auto const& result : results_) {
      if (result->dominates(*label)) {
        return true;
      }
    }
    return false;
  }

  void filter_results() {
    bool restart = false;
    for (auto it = std::begin(results_); it != std::end(results_);
         it = restart ? std::begin(results_) : std::next(it)) {
      restart = false;
      std::size_t size_before = results_.size();
      results_.erase(
          std::remove_if(std::begin(results_), std::end(results_),
                         [it](Label const* l) {
                           return l == (*it) ? false
                                             : (*it)->dominates_post_search(*l);
                         }),
          std::end(results_));
      if (results_.size() != size_before) {
        restart = true;
      }
    }
  }

  station_node const* goal_;
  std::vector<std::vector<Label*>> node_labels_;
  std::priority_queue<Label*, std::vector<Label*>, compare_labels> queue_;
  std::vector<Label*> equals_;
  std::unordered_map<node const*, std::vector<edge>> additional_edges_;
  std::vector<Label*> results_;
  LowerBounds& lower_bounds_;
  memory_manager& label_store_;
  statistics stats_;
  std::size_t max_labels_;
};

}  // namespace routing
}  // namespace motis
