#pragma once

#include <queue>
#include <list>
#include <ostream>
#include <unordered_map>

#include "motis/routing/statistics.h"

namespace motis {

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

  pareto_dijkstra(int node_count, station_node const* goal,
                  std::vector<Label*> const& start_labels,
                  std::unordered_map<node const*, std::vector<edge>> const&
                      additional_edges,
                  LowerBounds& lower_bounds, memory_manager<Label>& label_store)
      : _goal(goal),
        _node_labels(node_count),
        _queue(std::begin(start_labels), std::end(start_labels)),
        _additional_edges(additional_edges),
        _lower_bounds(lower_bounds),
        _label_store(label_store) {
    for (auto const& start_label : start_labels) {
      _node_labels[start_label->_node->_id].emplace_back(start_label);
    }
  }

  std::vector<Label*>& search() {
    _stats.start_label_count = _queue.size();
    _stats.labels_created = _label_store.used_size();

    while (!_queue.empty() || _equals.size() != 0) {
      if ((_stats.labels_created > (MAX_LABELS / 2) && _results.empty()) ||
          _stats.labels_created > MAX_LABELS) {
        _stats.max_label_quit = true;
        filter_results();
        return _results;
      }

      // get best label
      Label* label;
      if (_equals.size() > 0) {
        label = _equals.back();
        _equals.pop_back();
        _stats.labels_equals_popped++;
      } else {
        label = _queue.top();
        _stats.priority_queue_max_size =
            std::max(_stats.priority_queue_max_size, (int)_queue.size());
        _queue.pop();
        _stats.labels_popped++;
        _stats.labels_popped_after_last_result++;
      }

      // is label already made obsolete
      if (label->_dominated) {
        _stats.labels_dominated_by_later_labels++;
        continue;
      }

      if (dominated_by_results(label)) {
        _stats.labels_dominated_by_results++;
        continue;
      }

      if (_goal == label->_node->_station_node) continue;

      auto it = _additional_edges.find(label->_node);
      if (it != std::end(_additional_edges)) {
        for (auto const& additional_edge : it->second) {
          create_new_label(label, additional_edge);
        }
      }

      for (auto const& edge : label->_node->_edges) {
        create_new_label(label, edge);
      }
    }

    filter_results();
    return _results;
  }

  statistics get_statistics() const;

private:
  void create_new_label(Label* l, edge const& edge) {
    // use the edge to generate a new label
    Label blank;
    bool created = l->create_label(blank, edge, _lower_bounds);
    if (!created) {
      return;
    }

    auto new_label = new (label_store.create()) Label(blank);
    ++_stats.labels_created;

    if (edge.get_destination() == _goal) {
      add_result(new_label);
      if (_stats.labels_popped_until_first_result == -1) {
        _stats.labels_popped_until_first_result = _stats.labels_popped;
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
          _queue.push(new_label);
        } else {
          _equals.push_back(new_label);
        }
      } else {
        _stats.labels_dominated_by_former_labels++;
      }
    } else {
      _stats.labels_dominated_by_results++;
    }
  }

  bool add_result(Label* terminal_label) {
    for (auto it = _results.begin(); it != _results.end();) {
      Label* o = *it;
      if (terminal_label->dominates(*o, true)) {
        it = _results.erase(it);
      } else if (o->dominates(*terminal_label, true)) {
        return false;
      } else {
        ++it;
      }
    }
    _results.push_back(terminal_label);
    _stats.labels_popped_after_last_result = 0;
    return true;
  }

  bool add_label_to_node(Label* new_label, node const* dest) {
    auto& dest_labels = _node_labels[dest->_id];
    for (auto it = dest_labels.begin(); it != dest_labels.end();) {
      Label* o = *it;
      if (o->dominates(*new_label, false)) {
        return false;
      }

      if (new_label->dominates(*o, false)) {
        it = dest_labels.erase(it);
        o->_dominated = true;
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
    for (auto const& result : _results) {
      if (result->dominates(*label, true)) {
        return true;
      }
    }
    return false;
  }

  void filter_results() {
    bool restart = false;
    for (auto it = std::begin(_results); it != std::end(_results);
         it = restart ? std::begin(_results) : std::next(it)) {
      restart = false;
      std::size_t size_before = _results.size();
      _results.erase(std::remove_if(std::begin(_results), std::end(_results),
                                    [it](Label const* l) {
                                      return l == (*it)
                                                 ? false
                                                 : (*it)->dominates_hard(*l);
                                    }),
                     std::end(_results));
      if (_results.size() != size_before) {
        restart = true;
      }
    }
  }

  station_node const* _goal;
  std::vector<std::vector<Label*>> _node_labels;
  std::priority_queue<Label*, std::vector<Label*>, compare_labels> _queue;
  std::vector<Label*> _equals;
  std::unordered_map<node const*, std::vector<edge>> _additional_edges;
  std::vector<Label*> _results;
  LowerBounds& _lower_bounds;
  memory_manager<Label>& _label_store;
  statistics _stats;
};

}  // namespace motis
