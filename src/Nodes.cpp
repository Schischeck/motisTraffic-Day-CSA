#include "Nodes.h"

#include <algorithm>

#include "Edges.h"

namespace td {

std::vector<Node*> StationNode::getRouteNodes()
{
  std::vector<Node*> routeNodes;

  for (auto& edge : _edges)
    if (edge._to->isRouteNode())
      routeNodes.emplace_back(edge._to);

  return routeNodes;
}

int StationNode::addFootEdge(int nodeId, Edge fe)
{
  if(_footNode == nullptr)
  {
    _footNode = new Node(this, nodeId++);
    for (auto& routeNode : getRouteNodes())
      //check whether it is allowed to transfer at the route-node
      //we do this by checking, whether it has an edge to the station
      for (auto const& edge : routeNode->_edges)
        if(edge.getDestination() == this)
        {
          //The foot-edge may only be used if a train was used beforewards when
          //trying to use it from a route node
          routeNode->_edges.push_back(makeAfterTrainEdge(_footNode, 0, true));
          break;
        }
    _edges.emplace_back(makeFootEdge(_footNode));
  }
  _footNode->_edges.emplace_back(std::move(fe));

  return nodeId;
}

std::vector<std::pair<int /* to node id */, int /* train class */>>
  StationNode::getStationGraphEdges() const
{
  std::vector<StationGraphEdge> edges;

  for (auto const& edge : _edges)
  {
    // Destination can be:
    // - a route node of this station
    // - the foot node of this station
    Node const* edgeDest = edge.getDestination();

    // Destination of edges for route/foot node can be:
    // - for route nodes: back to station node (this station)
    // - for route nodes: route edge to another station
    // - for foot nodes: station node of another station
    for (auto const& edgeDestEdge : edgeDest->_edges)
    {
      // Skip foot edges and empty route edges.
      if (edgeDestEdge.empty())
        continue;

      // For a route edge:
      edges.emplace_back(
          edgeDestEdge.getDestination()->getStation()->_id,
          edgeDestEdge._m._routeEdge._conns[0]._fullCon->clasz);
    }
  }

  std::sort(std::begin(edges), std::end(edges));
  edges.erase(
      std::unique(std::begin(edges), std::end(edges)),
      std::end(edges));

  return edges;
}

}  // namespace td