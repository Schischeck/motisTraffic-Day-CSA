#pragma once

#include <cassert>
#include <vector>
#include <algorithm>

#include "motis/core/schedule/time.h"
#include "motis/core/schedule/connection.h"
#include "motis/core/common/array.h"
#include "motis/core/common/pointer.h"

namespace motis {

class node;

struct edge_cost {
  edge_cost(uint16_t time, light_connection const* c)
      : connection(c), time(time), price(0), transfer(false), slot(0) {}

  edge_cost(uint16_t time, bool transfer = false, uint16_t price = 0,
            uint8_t slot = 0)
      : connection(nullptr),
        time(time),
        price(price),
        transfer(transfer),
        slot(slot) {}

  uint16_t operator[](int index) const {
    switch (index) {
      case 0: return time;
      case 1: return transfer ? 1 : 0;
      case 2: return price;
      default: return 0;
    }
  }

  bool operator==(const edge_cost& o) const {
    return o.time == time && o.price == price && o.transfer == transfer &&
           o.slot == slot;
  }

  light_connection const* connection;
  uint16_t time;
  uint16_t price;
  bool transfer;
  uint8_t slot;
};

const edge_cost NO_EDGE(-1);

class edge {
public:
  enum type {
    INVALID_EDGE,
    ROUTE_EDGE,
    FOOT_EDGE,
    AFTER_TRAIN_FOOT_EDGE,
    MUMO_EDGE
  };

  edge() = default;

  /** route edge constructor. */
  edge(node* to, std::vector<light_connection> const& connections) : _to(to) {
    _m._type = ROUTE_EDGE;
    if (!connections.empty()) {
      _m._route_edge.init_empty();
      _m._route_edge._conns.set(std::begin(connections), std::end(connections));
      std::sort(std::begin(_m._route_edge._conns),
                std::end(_m._route_edge._conns));
    }
  }

  /** foot edge constructor. */
  edge(node* to, uint8_t type, uint16_t time_cost, uint16_t price,
       bool transfer, uint8_t slot = 0)
      : _to(to) {
    _m._type = type;
    _m._foot_edge._time_cost = time_cost;
    _m._foot_edge._price = price;
    _m._foot_edge._transfer = transfer;
    _m._foot_edge._slot = slot;

    assert(_m._type != type::ROUTE_EDGE);
  }

  edge_cost get_edge_cost(time start_time,
                          light_connection const* last_con) const {
    switch (_m._type) {
      case ROUTE_EDGE: return get_route_edge_cost(start_time);

      case AFTER_TRAIN_FOOT_EDGE:
        if (last_con == nullptr) return NO_EDGE;
      case MUMO_EDGE:
      case FOOT_EDGE:
        return edge_cost(_m._foot_edge._time_cost, _m._foot_edge._transfer,
                         _m._foot_edge._price, _m._foot_edge._slot);

      default: return NO_EDGE;
    }
  }

  edge_cost get_minimum_cost() const {
    if (_m._type == ROUTE_EDGE) {
      if (_m._route_edge._conns.size() == 0)
        return NO_EDGE;
      else {
        return edge_cost(
            std::min_element(
                std::begin(_m._route_edge._conns),
                std::end(_m._route_edge._conns),
                [](light_connection const& c1, light_connection const& c2) {
                  return c1.travel_time() < c2.travel_time();
                })->travel_time(),
            false, std::begin(_m._route_edge._conns)->_full_con->price);
      }
    } else if (_m._type == FOOT_EDGE || _m._type == AFTER_TRAIN_FOOT_EDGE)
      return edge_cost(0, _m._foot_edge._transfer);
    else
      return edge_cost(0);
  }

  light_connection const* get_connection(time const start_time) const {
    if (_m._route_edge._conns.size() == 0) return nullptr;

    auto it = std::lower_bound(std::begin(_m._route_edge._conns),
                               std::end(_m._route_edge._conns),
                               light_connection(start_time));

    return (it == std::end(_m._route_edge._conns)) ? nullptr : it;
  }

  edge_cost get_route_edge_cost(time const start_time) const {
    light_connection const* c = get_connection(start_time);
    return (c == nullptr) ? NO_EDGE : edge_cost(c->a_time - start_time, c);
  }

  inline node const* get_destination() const { return _to; }
  inline node* get_destination() { return _to; }

  inline bool valid() const { return type() != INVALID_EDGE; }

  inline uint8_t type() const { return _m._type; }

  inline char const* type_str() const {
    switch (type()) {
      case ROUTE_EDGE: return "ROUTE_EDGE";
      case FOOT_EDGE: return "FOOT_EDGE";
      case AFTER_TRAIN_FOOT_EDGE: return "AFTER_TRAIN_FOOT_EDGE";
      case MUMO_EDGE: return "MUMO_EDGE";
      default: return "INVALID";
    }
  }

  inline bool empty() const {
    return (type() != ROUTE_EDGE) ? true : _m._route_edge._conns.empty();
  }

  pointer<node> _to;
  pointer<node> _from;

  union edge_details {
    edge_details() { std::memset(this, 0, sizeof(*this)); }

    edge_details(edge_details&& other) {
      _type = other._type;
      if (_type == ROUTE_EDGE) {
        _route_edge.init_empty();
        _route_edge = std::move(other._route_edge);
      } else {
        _foot_edge = std::move(other._foot_edge);
      }
    }

    edge_details(edge_details const& other) {
      _type = other._type;
      if (_type == ROUTE_EDGE) {
        _route_edge.init_empty();
        _route_edge = other._route_edge;
      } else {
        _foot_edge.init_empty();
        _foot_edge = other._foot_edge;
      }
    }

    edge_details& operator=(edge_details&& other) {
      _type = other._type;
      if (_type == ROUTE_EDGE) {
        _route_edge.init_empty();
        _route_edge = std::move(other._route_edge);
      } else {
        _foot_edge.init_empty();
        _foot_edge = std::move(other._foot_edge);
      }

      return *this;
    }

    edge_details& operator=(edge_details const& other) {
      _type = other._type;
      if (_type == ROUTE_EDGE) {
        _route_edge.init_empty();
        _route_edge = other._route_edge;
      } else {
        _foot_edge.init_empty();
        _foot_edge = other._foot_edge;
      }

      return *this;
    }

    ~edge_details() {
      if (_type == ROUTE_EDGE) _route_edge._conns.~array<light_connection>();
    }

    // placeholder
    uint8_t _type;

    // TYPE = ROUTE_EDGE
    struct {
      uint8_t _type_padding;
      array<light_connection> _conns;

      void init_empty() { new (&_conns) array<light_connection>(); }
    } _route_edge;

    // TYPE = FOOT_EDGE & CO
    struct {
      uint8_t _type_padding;

      // edge weight
      uint16_t _time_cost;
      uint16_t _price;
      bool _transfer;

      // slot for mumo edge
      uint8_t _slot;

      void init_empty() {
        _time_cost = 0;
        _price = 0;
        _transfer = false;
        _slot = 0;
      }
    } _foot_edge;
  } _m;
};

/* convenience helper functions to generate the right edge type */

inline edge make_route_edge(node* to,
                            std::vector<light_connection> const& connections) {
  return edge(to, connections);
}

inline edge make_foot_edge(node* to, uint16_t time_cost = 0,
                           bool transfer = false) {
  return edge(to, edge::FOOT_EDGE, time_cost, 0, transfer);
}

inline edge make_after_train_edge(node* to, uint16_t time_cost = 0,
                                  bool transfer = false) {
  return edge(to, edge::AFTER_TRAIN_FOOT_EDGE, time_cost, 0, transfer);
}

inline edge make_mumo_edge(node* to, uint16_t time_cost = 0, uint16_t price = 0,
                           uint8_t slot = 0) {
  return edge(to, edge::MUMO_EDGE, time_cost, price, false, slot);
}

inline edge make_invalid_edge(node* to) {
  return edge(to, edge::INVALID_EDGE, 0, 0, false, 0);
}

}  // namespace motis
