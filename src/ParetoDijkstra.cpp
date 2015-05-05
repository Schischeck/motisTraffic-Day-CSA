#include "ParetoDijkstra.h"

#include "Graph.h"
#include "Station.h"

const bool DOMINANCE = true;
const bool FORWARDING = true;

using namespace td;

ParetoDijkstra::ParetoDijkstra(
    int nodeCount,
    StationNode const* goal,
    std::vector<Label*> const& startLabels,
    std::unordered_map<Node const*, std::vector<Edge>> const& additionalEdges,
    LowerBounds& lowerBounds,
    MemoryManager<Label>& labelStore)
  : _goal(goal),
    _nodeLabels(nodeCount),
    _queue(std::begin(startLabels), std::end(startLabels)),
    _additionalEdges(additionalEdges),
    _lowerBounds(lowerBounds),
    _labelStore(labelStore)
{
  for (auto const& startLabel : startLabels)
    _nodeLabels[startLabel->_node->_id].emplace_back(startLabel);
}

ParetoDijkstra::Statistics ParetoDijkstra::getStatistics() const
{ return _stats; }

std::vector<Label*>& ParetoDijkstra::search()
{
  _stats = Statistics();
  _stats.startLabelCount = _queue.size();
  _stats.labelsCreated = _labelStore.usedSize();

  while(!_queue.empty() || _equals.size() != 0)
  {
    if ((_stats.labelsCreated > (MAX_LABELS / 2) && _results.empty())
        || _stats.labelsCreated > MAX_LABELS)
    {
      _stats.maxLabelQuit = true;
      filterResults();
      return _results;
    }

    //get best label
    Label* label;
    if(_equals.size() > 0)
    {
      label = _equals.back();
      _equals.pop_back();
      _stats.labelsEqualsPopped++;
    }
    else
    {
      label = _queue.top();
      _stats.priorityQueueMaxSize = std::max(_stats.priorityQueueMaxSize,
                                             (int)_queue.size());
      _queue.pop();
      _stats.labelsPopped++;
      _stats.labelsPoppedAfterLastResult++;
    }

    //is label already made obsolete
    if(label->_dominated)
    {
      _stats.labelsDominatedByLaterLabels++;
      continue;
    }

    if(DOMINANCE && dominatedByResults(label))
    {
      _stats.labelsDominatedByResults++;
      continue;
    }

    if (_goal == label->_node->_stationNode)
      continue;

    auto it = _additionalEdges.find(label->_node);
    if (it != std::end(_additionalEdges))
      for (auto const& additionalEdge : it->second)
        createNewLabel(label, additionalEdge);

    for (auto const& edge : label->_node->_edges)
      createNewLabel(label, edge);
  }

  filterResults();
  return _results;
}

void ParetoDijkstra::createNewLabel(Label* label, Edge const& edge)
{
  //use the edge to generate a new label
  Label* newLabel = label->createLabel(edge, _lowerBounds, _labelStore);

  if(newLabel == nullptr)
    return;

  ++_stats.labelsCreated;

  if(edge.getDestination() == _goal)
  {
    addResult(newLabel);
    if(_stats.labelsPoppedUntilFirstResult == -1)
      _stats.labelsPoppedUntilFirstResult = _stats.labelsPopped;
    return;
  }

  //if the label is not dominated by a former one for the same node...
  //...add it to the queue
  if(!DOMINANCE || !dominatedByResults(newLabel))
  {
    if(addLabelToNode(newLabel, edge.getDestination()))
    {
      //if the newLabel is as good as label we don't have to push it into
      //the queue
      if(!FORWARDING || label < newLabel)
        _queue.push(newLabel);
      else
        _equals.push_back(newLabel);
    }
    else
      _stats.labelsDominatedByFormerLabels++;
  }
  else
    _stats.labelsDominatedByResults++;
}

bool ParetoDijkstra::addResult(Label* label)
{
  for(auto it = _results.begin(); it != _results.end();)
  {
    Label* o = *it;
    if(label->dominates(*o, true))
      it = _results.erase(it);
    else if(o->dominates(*label, true))
      return false;
    else
      ++it;
  }
  _results.push_back(label);
  _stats.labelsPoppedAfterLastResult = 0;
  return true;
}

bool ParetoDijkstra::dominatedByResults(Label* label)
{
  for (auto const& result : _results)
    if(result->dominates(*label, true))
      return true;
  return false;
}

bool ParetoDijkstra::addLabelToNode(Label* label, Node const* dest)
{
  auto& destLabels = _nodeLabels[dest->_id];
  for(auto it = destLabels.begin(); it != destLabels.end(); )
  {
    Label* o = *it;
    if(o->dominates(*label, false))
      return false;

    if(label->dominates(*o, false))
    {
      it = destLabels.erase(it);
      o->_dominated = true;
    }
    else
      ++it;
  }

  //it is very important for the performance to push front here
  //because earlier labels tend not to dominate later ones (not comparable)
  destLabels.insert(std::begin(destLabels), label);
  return true;
}

void ParetoDijkstra::filterResults() {
  bool restart = false;
  for (auto it = std::begin(_results); it != std::end(_results);
       it = restart ? std::begin(_results) : std::next(it))
  {
    restart = false;
    std::size_t sizeBefore = _results.size();
    _results.erase(
      std::remove_if(
        std::begin(_results), std::end(_results),
        [it](Label const* l) {
          return l == (*it) ? false : (*it)->dominatesHard(*l);
        }),
      std::end(_results)
    );
    if (_results.size() != sizeBefore)
      restart = true;
  }
}
