#ifndef TDNODES_H
#define TDNODES_H TDNODES_H

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <vector>

#include "TDTime.h"
#include "Array.h"
#include "Edges.h"
#include "serialization/Pointer.h"
#include "serialization/Deleter.h"

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
        _id(nodeId) {
  }

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

  std::vector<Node*> getRouteNodes();

  int addFootEdge(int nodeId, Edge fe);

  // (to station node, train class) pair
  typedef std::pair<int, int> StationGraphEdge;
  std::vector<StationGraphEdge> getStationGraphEdges() const;

  Pointer<Node> _footNode;
};

typedef std::unique_ptr<StationNode, Deleter<StationNode>> StationNodePtr;

}

#endif //TDNODES_H

