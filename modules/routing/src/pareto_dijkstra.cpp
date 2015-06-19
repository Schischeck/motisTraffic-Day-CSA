#include "motis/routing/pareto_dijkstra.h"

#include "motis/core/schedule/station.h"

#include "motis/routing/search.h"

const bool DOMINANCE = true;
const bool FORWARDING = true;

using namespace td;

pareto_dijkstra::pareto_dijkstra(
    int node_count,
    station_node const* goal,
    std::vector<label*> const& start_labels,
    std::unordered_map<node const*, std::vector<edge>> const& additional_edges,
    lower_bounds& lower_bounds,
    memory_manager<label>& label_store)
  : _goal(goal),
    _node_labels(node_count),
    _queue(std::begin(start_labels), std::end(start_labels)),
    _additional_edges(additional_edges),
    _lower_bounds(lower_bounds),
    _label_store(label_store)
{
  for (auto const& start_label : start_labels)
    _node_labels[start_label->_node->_id].emplace_back(start_label);
}

pareto_dijkstra::statistics pareto_dijkstra::get_statistics() const
{ return _stats; }

std::vector<label*>& pareto_dijkstra::search()
{
  _stats = statistics();
  _stats.start_label_count = _queue.size();
  _stats.labels_created = _label_store.used_size();

  while(!_queue.empty() || _equals.size() != 0)
  {
    if ((_stats.labels_created > (MAX_LABELS / 2) && _results.empty())
        || _stats.labels_created > MAX_LABELS)
    {
      _stats.max_label_quit = true;
      filter_results();
      return _results;
    }

    //get best label
    label* label;
    if(_equals.size() > 0)
    {
      label = _equals.back();
      _equals.pop_back();
      _stats.labels_equals_popped++;
    }
    else
    {
      label = _queue.top();
      _stats.priority_queue_max_size = std::max(_stats.priority_queue_max_size,
                                             (int)_queue.size());
      _queue.pop();
      _stats.labels_popped++;
      _stats.labels_popped_after_last_result++;
    }

    //is label already made obsolete
    if(label->_dominated)
    {
      _stats.labels_dominated_by_later_labels++;
      continue;
    }

    if(DOMINANCE && dominated_by_results(label))
    {
      _stats.labels_dominated_by_results++;
      continue;
    }

    if (_goal == label->_node->_station_node)
      continue;

    auto it = _additional_edges.find(label->_node);
    if (it != std::end(_additional_edges))
      for (auto const& additional_edge : it->second)
        create_new_label(label, additional_edge);

    for (auto const& edge : label->_node->_edges)
      create_new_label(label, edge);
  }

  filter_results();
  return _results;
}

void pareto_dijkstra::create_new_label(label* l, edge const& edge)
{
  //use the edge to generate a new label
  label* new_label = l->create_label(edge, _lower_bounds, _label_store);

  if(new_label == nullptr)
    return;

  ++_stats.labels_created;

  if(edge.get_destination() == _goal)
  {
    add_result(new_label);
    if(_stats.labels_popped_until_first_result == -1)
      _stats.labels_popped_until_first_result = _stats.labels_popped;
    return;
  }

  //if the label is not dominated by a former one for the same node...
  //...add it to the queue
  if(!DOMINANCE || !dominated_by_results(new_label))
  {
    if(add_label_to_node(new_label, edge.get_destination()))
    {
      //if the new_label is as good as label we don't have to push it into
      //the queue
      if(!FORWARDING || l < new_label)
        _queue.push(new_label);
      else
        _equals.push_back(new_label);
    }
    else
      _stats.labels_dominated_by_former_labels++;
  }
  else
    _stats.labels_dominated_by_results++;
}

bool pareto_dijkstra::add_result(label* terminal_label)
{
  for(auto it = _results.begin(); it != _results.end();)
  {
    label* o = *it;
    if(terminal_label->dominates(*o, true))
      it = _results.erase(it);
    else if(o->dominates(*terminal_label, true))
      return false;
    else
      ++it;
  }
  _results.push_back(terminal_label);
  _stats.labels_popped_after_last_result = 0;
  return true;
}

bool pareto_dijkstra::dominated_by_results(label* label)
{
  for (auto const& result : _results)
    if(result->dominates(*label, true))
      return true;
  return false;
}

bool pareto_dijkstra::add_label_to_node(label* new_label, node const* dest)
{
  auto& dest_labels = _node_labels[dest->_id];
  for(auto it = dest_labels.begin(); it != dest_labels.end(); )
  {
    label* o = *it;
    if(o->dominates(*new_label, false))
      return false;

    if(new_label->dominates(*o, false))
    {
      it = dest_labels.erase(it);
      o->_dominated = true;
    }
    else
      ++it;
  }

  //it is very important for the performance to push front here
  //because earlier labels tend not to dominate later ones (not comparable)
  dest_labels.insert(std::begin(dest_labels), new_label);
  return true;
}

void pareto_dijkstra::filter_results() {
  bool restart = false;
  for (auto it = std::begin(_results); it != std::end(_results);
       it = restart ? std::begin(_results) : std::next(it))
  {
    restart = false;
    std::size_t size_before = _results.size();
    _results.erase(
      std::remove_if(
        std::begin(_results), std::end(_results),
        [it](label const* l) {
          return l == (*it) ? false : (*it)->dominates_hard(*l);
        }),
      std::end(_results)
    );
    if (_results.size() != size_before)
      restart = true;
  }
}
