#ifndef TDEDGES_H
#define TDEDGES_H TDEDGES_H

#include <cassert>
#include <vector>
#include <algorithm>

#include "motis/core/schedule/Time.h"
#include "motis/core/schedule/Connection.h"
#include "motis/core/common/Array.h"
#include "motis/core/common/Pointer.h"

namespace td
{

class Node;

struct EdgeCost
{
  EdgeCost(int time, LightConnection const* c)
      : connection(c),
        time(time),
        price(0),
        transfer(false),
        slot(0)
  {}

  EdgeCost(uint16_t time, bool transfer = false, uint16_t price = 0, uint8_t slot = 0)
      : connection(nullptr),
        time(time),
        price(price),
        transfer(transfer),
        slot(slot)
  {}

  int operator[](int index) const
  {
    switch(index)
    {
      case 0:
        return time;
      case 1:
        return transfer ? 1 : 0;
      case 2:
        return price;
      default:
        return 0;
    }
  }

  bool operator==(const EdgeCost& o) const
  {
    return o.time == time &&
           o.price == price &&
           o.transfer == transfer &&
           o.slot == slot;
  }

  LightConnection const* connection;
  uint16_t time;
  uint16_t price;
  bool transfer;
  uint8_t slot;
};

const EdgeCost NO_EDGE(-1);

class Edge
{
public:
  Edge() = default;

  /** Route Edge constructor. */
  Edge(Node* to, std::vector<LightConnection> const& connections)
      : _to(to) {
    _m._type = ROUTE_EDGE;
    if (!connections.empty())
    {
      _m._routeEdge.initEmpty();
      _m._routeEdge._conns.set(std::begin(connections), std::end(connections));
      std::sort(std::begin(_m._routeEdge._conns),
                std::end(_m._routeEdge._conns));
    }
  }

  /** Foot Edge constructor. */
  Edge(Node* to,
       uint8_t type,
       uint16_t timeCost, uint16_t price, bool transfer,
       uint8_t slot = 0)
      : _to(to)
  {
    _m._type = type;
    _m._footEdge._timeCost = timeCost;
    _m._footEdge._price = price;
    _m._footEdge._transfer = transfer;
    _m._footEdge._slot = slot;

    assert(_m._type != Type::ROUTE_EDGE);
  }

  EdgeCost getEdgeCost(Time startTime, LightConnection const* lastCon) const
  {
    switch (_m._type)
    {
      case Type::ROUTE_EDGE:
        return getRouteEdgeCost(startTime);

      case Type::AFTER_TRAIN_FOOT_EDGE:
        if (lastCon == nullptr)
          return NO_EDGE;
      case Type::MUMO_EDGE:
      case Type::FOOT_EDGE:
        return EdgeCost(
          _m._footEdge._timeCost,
          _m._footEdge._transfer,
          _m._footEdge._price,
          _m._footEdge._slot
        );

      default:
        return NO_EDGE;
    }
  }

  EdgeCost getMinimumCost() const
  {
    if (_m._type == ROUTE_EDGE)
    {
      if(_m._routeEdge._conns.size() == 0)
        return NO_EDGE;
      else
      {
        return EdgeCost(
          std::min_element(
            std::begin(_m._routeEdge._conns),
            std::end(_m._routeEdge._conns),
            [](LightConnection const& c1, LightConnection const& c2) {
              return c1.travelTime() < c2.travelTime();
            }
          )->travelTime(),
          false,
          std::begin(_m._routeEdge._conns)->_fullCon->price
        );
      }
    }
    else if (_m._type == FOOT_EDGE || _m._type == AFTER_TRAIN_FOOT_EDGE)
      return EdgeCost(0, _m._footEdge._transfer);
    else
      return EdgeCost(0);
  }

  LightConnection const* getConnection(Time const startTime) const
  {
    if(_m._routeEdge._conns.size() == 0)
      return nullptr;

    auto it = std::lower_bound(
        std::begin(_m._routeEdge._conns),
        std::end(_m._routeEdge._conns),
        LightConnection(startTime)
    );

    return (it == std::end(_m._routeEdge._conns)) ? nullptr : it;
  }

  EdgeCost getRouteEdgeCost(Time const startTime) const
  {
    LightConnection const* c = getConnection(startTime);
    return (c == nullptr) ? NO_EDGE : EdgeCost(c->aTime - startTime, c);
  }

  inline Node const* getDestination() const { return _to; }

  inline bool valid() const { return type() != INVALID_EDGE; }

  inline uint8_t type() const { return _m._type; }

  inline bool empty() const
  { return (type() != ROUTE_EDGE) ? true : _m._routeEdge._conns.empty(); }

  enum Type {
    INVALID_EDGE,
    ROUTE_EDGE,
    FOOT_EDGE,
    AFTER_TRAIN_FOOT_EDGE,
    MUMO_EDGE
  };

  Pointer<Node> _to;

  union EdgeDetails
  {
    EdgeDetails()
    { std::memset(this, 0, sizeof(*this)); }

    EdgeDetails(EdgeDetails&& other)
    {
      _type = other._type;
      if (_type == ROUTE_EDGE)
      {
        _routeEdge.initEmpty();
        _routeEdge = std::move(other._routeEdge);
      }
      else
      {
        _footEdge = std::move(other._footEdge);
      }
    }

    EdgeDetails(EdgeDetails const& other)
    {
      _type = other._type;
      if (_type == ROUTE_EDGE)
      {
        _routeEdge.initEmpty();
        _routeEdge = other._routeEdge;
      }
      else
      {
        _footEdge.initEmpty();
        _footEdge = other._footEdge;
      }
    }

    EdgeDetails& operator = (EdgeDetails&& other)
    {
      _type = other._type;
      if (_type == ROUTE_EDGE)
      {
        _routeEdge.initEmpty();
        _routeEdge = std::move(other._routeEdge);
      }
      else
      {
        _footEdge.initEmpty();
        _footEdge = std::move(other._footEdge);
      }

      return *this;
    }

    EdgeDetails& operator = (EdgeDetails const& other)
    {
      _type = other._type;
      if (_type == ROUTE_EDGE)
      {
        _routeEdge.initEmpty();
        _routeEdge = other._routeEdge;
      }
      else
      {
        _footEdge.initEmpty();
        _footEdge = other._footEdge;
      }

      return *this;
    }

    ~EdgeDetails()
    {
      if (_type == ROUTE_EDGE)
        _routeEdge._conns.~Array<LightConnection>();
    }

    // placeholder
    uint8_t _type;

    // TYPE = ROUTE_EDGE
    struct {
      uint8_t _typePadding;
      Array<LightConnection> _conns;

      void initEmpty() { new (&_conns) Array<LightConnection>(); }
    } _routeEdge;

    // TYPE = FOOT_EDGE & CO
    struct {
      uint8_t _typePadding;

      // edge weight
      uint16_t _timeCost;
      uint16_t _price;
      bool _transfer;

      // slot for mumo edge
      uint8_t _slot;

      void initEmpty()
      {
        _timeCost = 0;
        _price = 0;
        _transfer = false;
        _slot = 0;
      }
    } _footEdge;
  } _m;
};

/* Convenience helper functions to generate the right edge type */

inline Edge makeRouteEdge(Node* to,
                          std::vector<LightConnection> const& connections)
{ return Edge(to, connections); }

inline Edge makeFootEdge(Node* to,
                         uint16_t timeCost = 0,
                         bool transfer = false)
{
  return Edge(to,
              Edge::Type::FOOT_EDGE,
              timeCost, 0, transfer);
}

inline Edge makeAfterTrainEdge(Node* to, uint16_t timeCost = 0, bool transfer = false)
{
  return Edge(to,
              Edge::Type::AFTER_TRAIN_FOOT_EDGE,
              timeCost, 0, transfer);
}

inline Edge makeMumoEdge(
    Node* to,
    uint16_t timeCost = 0, uint16_t price = 0, uint8_t slot = 0)
{
  return Edge(to,
              Edge::Type::MUMO_EDGE,
              timeCost, price, false, slot);
}

inline Edge makeInvalidEdge(Node* to)
{ return Edge(to, Edge::Type::INVALID_EDGE, 0, 0, false, 0); }

}  // namespace td

#endif //TDEDGES_H

