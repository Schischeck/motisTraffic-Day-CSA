#ifndef TD_CONSTANT_GRAPH
#define TD_CONSTANT_GRAPH

#include <queue>
#include <vector>
#include <unordered_map>
#include <array>

#include "motis/core/schedule/Nodes.h"

namespace td
{

struct SimpleEdge
{
  SimpleEdge() : to(0) {}
  SimpleEdge(int to, EdgeCost cost)
      : to(to),
        dist({{cost.time, cost.transfer, cost.price}})
  {}

  bool operator<(const SimpleEdge& o) const
  {
    if(to < o.to)
      return true;
    else if(to == o.to)
      return dist < o.dist;
    return false;
  }

  uint32_t to;
  std::array<uint16_t, 3> dist;
};

class ConstantGraph
{
public:
  ConstantGraph() = default;

  ConstantGraph(std::vector<StationNodePtr> const& stationNodes)
  {
    for (auto const& stationNode : stationNodes)
    {
      addEdges(*stationNode.get());
      for(auto const& stationNodeEdge : stationNode->_edges)
        addEdges(*stationNodeEdge.getDestination());
    }
  }

  void addEdges(Node const& node)
  {
    for(auto const& edge : node._edges)
    {
      auto from = edge.getDestination()->_id;

      if (_edges.size() <= from)
        _edges.resize(from + 1);

      _edges[from].emplace_back(node._id, edge.getMinimumCost());
    }
  }

  int addEdge(int nodeIndex, const SimpleEdge& newEdge)
  {
    _edges[nodeIndex].push_back(newEdge);
    return _edges[nodeIndex].size() - 1;
  }

  std::vector<std::vector<SimpleEdge>> _edges;
};

template<int Criterion>
class ConstantGraphDijkstra
{
public:
  struct Label
  {
    Label(uint32_t node, uint32_t dist) : node(node), dist(dist) {}

    friend bool operator>(Label const& a, Label const& b)
    { return a.dist > b.dist; }

    uint32_t node, dist;
  };

  ConstantGraphDijkstra(
      ConstantGraph const& graph, int goal,
      std::unordered_map<int, std::vector<SimpleEdge>> const& additionalEdges)
    : _graph(graph),
      _additionalEdges(additionalEdges)
  {
    _dists.resize(_graph._edges.size(), std::numeric_limits<uint32_t>::max());
    _dists[goal] = 0;
    _pq.push(Label(goal, 0));
  }

  inline uint32_t getDistance(int node) const
  {
    assert(node < static_cast<int>(_dists.size()));
    return _dists[node];
  }

  void run()
  {
    while(!_pq.empty())
    {
      auto label = _pq.top();
      _pq.pop();

      for (auto const& edge : _graph._edges[label.node])
        expandEdge(label.dist, edge);

      auto additionalEdgesIt = _additionalEdges.find(label.node);
      if (additionalEdgesIt != std::end(_additionalEdges))
        for (auto const& edge : additionalEdgesIt->second)
          expandEdge(label.dist, edge);
    }
  }

  inline void expandEdge(uint32_t dist, SimpleEdge const& edge)
  {
    uint32_t newDist = dist + edge.dist[Criterion];
    if (newDist < _dists[edge.to])
    {
      _dists[edge.to] = newDist;
      _pq.push(Label(edge.to, newDist));
    }
  }

  ConstantGraph const& _graph;
  std::priority_queue<Label, std::vector<Label>, std::greater<Label>> _pq;
  std::vector<uint32_t> _dists;
  std::unordered_map<int, std::vector<SimpleEdge>> const& _additionalEdges;
};

}  // namespace td

#endif //TD_CONSTANT_GRAPH

