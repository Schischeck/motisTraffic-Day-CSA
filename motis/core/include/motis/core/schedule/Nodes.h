#ifndef TDNODES_H
#define TDNODES_H TDNODES_H

#include <cstring>
#include <cstdlib>
#include <memory>
#include <vector>

#include "motis/core/common/Array.h"
#include "motis/core/common/Pointer.h"
#include "motis/core/common/Deleter.h"
#include "motis/core/schedule/Edges.h"
#include "motis/core/schedule/Time.h"

namespace td
{

class StationNode;
class Node;
class Label;

class Node
{
public:
  Node(StationNode* stationNode, int nodeId)
      : _stationNode(stationNode),
        _route(-1),
        _id(nodeId)
  {}

  bool isStationNode() const
  { return _stationNode == nullptr; }

  bool isRouteNode() const
  { return _route != -1; }

  bool isFootNode() const
  { return !isStationNode() && !isRouteNode(); }

  char const* typeStr() const
  {
    if (isStationNode())
      return "STATION_NODE";
    else if (isRouteNode())
      return "ROUTE_NODE";
    else
      return "FOOT_NODE";
  }

  StationNode* asStationNode()
  {
    if (_stationNode == nullptr)
      return reinterpret_cast<StationNode*>(this);
    else
      return nullptr;
  }

  StationNode const* asStationNode() const
  {
    if (_stationNode == nullptr)
      return reinterpret_cast<StationNode const*>(this);
    else
      return nullptr;
  }

  StationNode* getStation()
  {
    if (_stationNode == nullptr)
      return reinterpret_cast<StationNode*>(this);
    else
      return _stationNode;
  }

  StationNode const* getStation() const
  {
    if (_stationNode == nullptr)
      return reinterpret_cast<StationNode const*>(this);
    else
      return _stationNode;
  }

  Array<Edge> _edges;
  Array<Pointer<Edge>> _incomingEdges;
  Pointer<StationNode> _stationNode;
  int32_t _route;
  uint32_t _id;
};

class StationNode : public Node
{
public:
  StationNode(int nodeId)
      : Node(nullptr, nodeId),
        _footNode(nullptr)
  {}

  ~StationNode()
  {
    for (auto& routeNode : getRouteNodes())
      delete routeNode;

    if (_footNode != nullptr)
      delete _footNode.ptr();
  }

  std::vector<Node*> getRouteNodes()
  {
    std::vector<Node*> routeNodes;

    for (auto& edge : _edges)
      if (edge._to->isRouteNode())
        routeNodes.emplace_back(edge._to);

    return routeNodes;
  }

  int addFootEdge(int nodeId, Edge fe)
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
            //The foot-edge may only be used
            //if a train was used beforewards when
            //trying to use it from a route node
            routeNode->_edges.push_back(makeAfterTrainEdge(_footNode, 0, true));
            break;
          }
      _edges.emplace_back(makeFootEdge(_footNode));
    }
    _footNode->_edges.emplace_back(std::move(fe));

    return nodeId;
  }

  Pointer<Node> _footNode;
};

typedef std::unique_ptr<StationNode, Deleter<StationNode>> StationNodePtr;

}

#endif //TDNODES_H

