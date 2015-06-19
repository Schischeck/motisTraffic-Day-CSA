#ifndef TDNODES_H
#define TDNODES_H TDNODES_H

#include <cstring>
#include <cstdlib>
#include <memory>
#include <vector>

#include "motis/core/common/array.h"
#include "motis/core/common/pointer.h"
#include "motis/core/common/deleter.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/time.h"

namespace td
{

class station_node;
class node;
class label;

class node
{
public:
  node(station_node* station_node, int node_id)
      : _station_node(station_node),
        _route(-1),
        _id(node_id)
  {}

  bool is_station_node() const
  { return _station_node == nullptr; }

  bool is_route_node() const
  { return _route != -1; }

  bool is_foot_node() const
  { return !is_station_node() && !is_route_node(); }

  char const* type_str() const
  {
    if (is_station_node())
      return "STATION_NODE";
    else if (is_route_node())
      return "ROUTE_NODE";
    else
      return "FOOT_NODE";
  }

  station_node* as_station_node()
  {
    if (_station_node == nullptr)
      return reinterpret_cast<station_node*>(this);
    else
      return nullptr;
  }

  station_node const* as_station_node() const
  {
    if (_station_node == nullptr)
      return reinterpret_cast<station_node const*>(this);
    else
      return nullptr;
  }

  station_node* get_station()
  {
    if (_station_node == nullptr)
      return reinterpret_cast<station_node*>(this);
    else
      return _station_node;
  }

  station_node const* get_station() const
  {
    if (_station_node == nullptr)
      return reinterpret_cast<station_node const*>(this);
    else
      return _station_node;
  }

  array<edge> _edges;
  array<pointer<edge>> _incoming_edges;
  pointer<station_node> _station_node;
  int32_t _route;
  uint32_t _id;
};

class station_node : public node
{
public:
  station_node(int node_id)
      : node(nullptr, node_id),
        _foot_node(nullptr)
  {}

  ~station_node()
  {
    for (auto& route_node : get_route_nodes())
      delete route_node;

    if (_foot_node != nullptr)
      delete _foot_node.ptr();
  }

  std::vector<node*> get_route_nodes()
  {
    std::vector<node*> route_nodes;

    for (auto& edge : _edges)
      if (edge._to->is_route_node())
        route_nodes.emplace_back(edge._to);

    return route_nodes;
  }

  int add_foot_edge(int node_id, edge fe)
  {
    if(_foot_node == nullptr)
    {
      _foot_node = new node(this, node_id++);
      for (auto& route_node : get_route_nodes())
        //check whether it is allowed to transfer at the route-node
        //we do this by checking, whether it has an edge to the station
        for (auto const& edge : route_node->_edges)
          if(edge.get_destination() == this)
          {
            //the foot-edge may only be used
            //if a train was used beforewards when
            //trying to use it from a route node
            route_node->_edges.push_back(make_after_train_edge(_foot_node, 0, true));
            break;
          }
      _edges.emplace_back(make_foot_edge(_foot_node));
    }
    _foot_node->_edges.emplace_back(std::move(fe));

    return node_id;
  }

  pointer<node> _foot_node;
};

typedef std::unique_ptr<station_node, deleter<station_node>> station_node_ptr;

}

#endif //TDNODES_H

