#pragma once

#include <cassert>
#include <vector>
#include <algorithm>

#include "motis/core/schedule/time.h"
#include "motis/core/schedule/connection.h"
#include "motis/core/common/array.h"
#include "motis/core/common/constants.h"

namespace motis {

class node;

struct edge_cost {
  edge_cost() : time(INVALID_TIME) {}

  edge_cost(duration time, light_connection const* c)
      : connection(c), time(time), price(0), transfer(false), slot(0) {}

  edge_cost(duration time, bool transfer = false, uint16_t price = 0,
            uint8_t slot = 0)
      : connection(nullptr),
        time(time),
        price(price),
        transfer(transfer),
        slot(slot) {}

  bool is_valid() const { return time != INVALID_TIME; }

  uint16_t operator[](int index) const {
    switch (index) {
      case 0: return time;
      case 1: return transfer ? 1 : 0;
      case 2: return price;
      default: return 0;
    }
  }

  light_connection const* connection;
  duration time;
  uint16_t price;
  bool transfer;
  uint8_t slot;
};

const edge_cost NO_EDGE = edge_cost();

class edge {
public:
  enum type {
    INVALID_EDGE,
    ROUTE_EDGE,
    FOOT_EDGE,
    AFTER_TRAIN_FOOT_EDGE,
    MUMO_EDGE,
    TIME_DEPENDENT_MUMO_EDGE,
    HOTEL_EDGE,
    THROUGH_EDGE
  };

  edge() = default;

  /** route edge constructor. */
  edge(node* from, node* to, std::vector<light_connection> const& connections)
      : _from(from), _to(to) {
    _m._type = ROUTE_EDGE;
    if (!connections.empty()) {
      _m._route_edge.init_empty();
      _m._route_edge._conns.set(std::begin(connections), std::end(connections));
      std::sort(std::begin(_m._route_edge._conns),
                std::end(_m._route_edge._conns));
    }
  }

  /** foot edge constructor. */
  edge(node* from, node* to, uint8_t type, uint16_t time_cost, uint16_t price,
       bool transfer, uint8_t slot = 0)
      : _from(from), _to(to) {
    _m._type = type;
    _m._foot_edge._time_cost = time_cost;
    _m._foot_edge._price = price;
    _m._foot_edge._transfer = transfer;
    _m._foot_edge._slot = slot;

    assert(_m._type != ROUTE_EDGE);
  }

  /** hotel edge constructor. */
  edge(node* station_node, uint16_t checkout_time, uint16_t min_stay_duration,
       uint16_t price)
      : _from(station_node), _to(station_node) {
    _m._type = HOTEL_EDGE;
    _m._hotel_edge._checkout_time = checkout_time;
    _m._hotel_edge._min_stay_duration = min_stay_duration;
    _m._hotel_edge._price = price;
  }

  edge_cost get_edge_cost(time start_time,
                          light_connection const* last_con) const {
    switch (_m._type) {
      case ROUTE_EDGE: return get_route_edge_cost(start_time);

      case AFTER_TRAIN_FOOT_EDGE:
        if (last_con == nullptr) {
          return NO_EDGE;
        }
      /* no break */
      case MUMO_EDGE:
      case FOOT_EDGE:
        return edge_cost(_m._foot_edge._time_cost, _m._foot_edge._transfer,
                         _m._foot_edge._price, _m._foot_edge._slot);
      case TIME_DEPENDENT_MUMO_EDGE: {
        unsigned const start_time_mod = start_time % 1440;
        if (start_time_mod >= LATE_TAXI_BEGIN_TIME ||
            start_time_mod <= LATE_TAXI_END_TIME) {
          return edge_cost(_m._foot_edge._time_cost, _m._foot_edge._transfer,
                           _m._foot_edge._price, _m._foot_edge._slot);
        } else {
          return NO_EDGE;
        }
      }
      case HOTEL_EDGE: {
        return edge_cost(calc_duration_hotel_edge(start_time), false,
                         _m._hotel_edge._price, 0);
      }

      case THROUGH_EDGE: return edge_cost(0, false, 0, 0);

      default: return NO_EDGE;
    }
  }

  edge_cost get_minimum_cost() const {
    if (_m._type == ROUTE_EDGE) {
      if (_m._route_edge._conns.size() == 0) {
        return NO_EDGE;
      } else {
        return edge_cost(
            std::min_element(
                std::begin(_m._route_edge._conns),
                std::end(_m._route_edge._conns),
                [](light_connection const& c1, light_connection const& c2) {
                  return c1.travel_time() < c2.travel_time();
                })
                ->travel_time(),
            false, std::begin(_m._route_edge._conns)->_full_con->price);
      }
    } else if (_m._type == FOOT_EDGE || _m._type == AFTER_TRAIN_FOOT_EDGE) {
      return edge_cost(0, _m._foot_edge._transfer);
    } else if (_m._type == HOTEL_EDGE) {
      return edge_cost(0, false, _m._hotel_edge._price);
    } else if (_m._type == MUMO_EDGE || _m._type == TIME_DEPENDENT_MUMO_EDGE) {
      return edge_cost(0, false, _m._foot_edge._price);
    } else {
      return edge_cost(0);
    }
  }

  light_connection const* get_connection(time const start_time) const {
    if (_m._route_edge._conns.size() == 0) {
      return nullptr;
    }

    auto it = std::lower_bound(std::begin(_m._route_edge._conns),
                               std::end(_m._route_edge._conns),
                               light_connection(start_time));

    return (it == std::end(_m._route_edge._conns)) ? nullptr : &*it;
  }

  light_connection const* get_connection_reverse(time const start_time) const {
    if (_m._route_edge._conns.size() == 0) {
      return nullptr;
    }

    auto it = std::lower_bound(
        _m._route_edge._conns.rbegin(), _m._route_edge._conns.rend(),
        light_connection(0, start_time, nullptr),
        [](light_connection const& lhs, light_connection const& rhs) {
          return lhs.a_time > rhs.a_time;
        });

    return (it == _m._route_edge._conns.rend()) ? nullptr : &*it;
  }

  light_connection* get_connection(time const start_time) {
    return const_cast<light_connection*>(
        static_cast<const edge*>(this)->get_connection(start_time));
  }

  light_connection* get_connection_reverse(time const start_time) {
    return const_cast<light_connection*>(
        static_cast<const edge*>(this)->get_connection_reverse(start_time));
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
      case TIME_DEPENDENT_MUMO_EDGE: return "TIME_DEPENDENT_MUMO_EDGE";
      case HOTEL_EDGE: return "HOTEL_EDGE";
      case THROUGH_EDGE: return "THROUGH_EDGE";
      default: return "INVALID";
    }
  }

  inline bool empty() const {
    return (type() != ROUTE_EDGE) ? true : _m._route_edge._conns.empty();
  }

  node* _from;
  node* _to;

  union edge_details {
    edge_details() { std::memset(this, 0, sizeof(*this)); }

    edge_details(edge_details&& other) {
      _type = other._type;
      if (_type == ROUTE_EDGE) {
        _route_edge.init_empty();
        _route_edge = std::move(other._route_edge);
      } else if (_type == HOTEL_EDGE) {
        _hotel_edge = std::move(other._hotel_edge);
      } else {
        _foot_edge = std::move(other._foot_edge);
      }
    }

    edge_details(edge_details const& other) {
      _type = other._type;
      if (_type == ROUTE_EDGE) {
        _route_edge.init_empty();
        _route_edge = other._route_edge;
      } else if (_type == HOTEL_EDGE) {
        _hotel_edge = std::move(other._hotel_edge);
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
      } else if (_type == HOTEL_EDGE) {
        _hotel_edge = std::move(other._hotel_edge);
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
      } else if (_type == HOTEL_EDGE) {
        _hotel_edge = std::move(other._hotel_edge);
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

    // TYPE = HOTEL_EDGE
    struct {
      uint8_t _type_padding;
      uint16_t _checkout_time;
      uint16_t _min_stay_duration;
      uint16_t _price;
    } _hotel_edge;
  } _m;

private:
  uint16_t calc_duration_hotel_edge(time const start_time) const {
    uint16_t offset =
        start_time % 1440 < _m._hotel_edge._checkout_time ? 0 : 1440;
    return std::max(
        _m._hotel_edge._min_stay_duration,
        static_cast<uint16_t>((_m._hotel_edge._checkout_time + offset) -
                              (start_time % 1440)));
  }
};

/* convenience helper functions to generate the right edge type */

inline edge make_route_edge(node* from, node* to,
                            std::vector<light_connection> const& connections) {
  return edge(from, to, connections);
}

inline edge make_foot_edge(node* from, node* to, uint16_t time_cost = 0,
                           bool transfer = false) {
  return edge(from, to, edge::FOOT_EDGE, time_cost, 0, transfer);
}

inline edge make_after_train_edge(node* from, node* to, uint16_t time_cost = 0,
                                  bool transfer = false) {
  return edge(from, to, edge::AFTER_TRAIN_FOOT_EDGE, time_cost, 0, transfer);
}

inline edge make_mumo_edge(node* from, node* to, uint16_t time_cost = 0,
                           uint16_t price = 0, uint8_t slot = 0) {
  return edge(from, to, edge::MUMO_EDGE, time_cost, price, false, slot);
}

inline edge make_time_dependent_mumo_edge(node* from, node* to,
                                          uint16_t time_cost = 0,
                                          uint16_t price = 0,
                                          uint8_t slot = 0) {
  return edge(from, to, edge::TIME_DEPENDENT_MUMO_EDGE, time_cost, price, false,
              slot);
}

inline edge make_hotel_edge(node* station_node, uint16_t checkout_time,
                            uint16_t min_stay_duration, uint16_t price) {
  return edge(station_node, checkout_time, min_stay_duration, price);
}

inline edge make_invalid_edge(node* from, node* to) {
  return edge(from, to, edge::INVALID_EDGE, 0, 0, false, 0);
}

inline edge make_through_edge(node* from, node* to) {
  return edge(from, to, edge::THROUGH_EDGE, 0, 0, false, 0);
}

}  // namespace motis
