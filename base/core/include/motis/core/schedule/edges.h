#pragma once

#include <cassert>
#include <algorithm>
#include <vector>

#include "motis/core/common/array.h"
#include "motis/core/common/constants.h"
#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/time.h"

namespace motis {

class node;

struct edge_cost {
  edge_cost() : time_(INVALID_TIME) {}

  edge_cost(duration time, light_connection const* c)
      : connection_(c), time_(time), price_(0), transfer_(false), slot_(0) {}

  explicit edge_cost(duration time, bool transfer = false, uint16_t price = 0,
                     uint8_t slot = 0)
      : connection_(nullptr),
        time_(time),
        price_(price),
        transfer_(transfer),
        slot_(slot) {}

  bool is_valid() const { return time_ != INVALID_TIME; }

  uint16_t operator[](int index) const {
    switch (index) {
      case 0: return time_;
      case 1: return transfer_ ? 1 : 0;
      case 2: return price_;
      default: return 0;
    }
  }

  light_connection const* connection_;
  duration time_;
  uint16_t price_;
  bool transfer_;
  uint8_t slot_;
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
    PERIODIC_MUMO_EDGE,
    TIME_DEPENDENT_MUMO_EDGE,
    HOTEL_EDGE,
    THROUGH_EDGE
  };

  edge() = default;

  /** route edge constructor. */
  edge(node* from, node* to, std::vector<light_connection> const& connections)
      : from_(from), to_(to) {
    m_.type_ = ROUTE_EDGE;
    if (!connections.empty()) {
      m_.route_edge_.init_empty();
      m_.route_edge_.conns_.set(std::begin(connections), std::end(connections));
      std::sort(std::begin(m_.route_edge_.conns_),
                std::end(m_.route_edge_.conns_));
    }
  }

  /** foot edge constructor. */
  edge(node* from, node* to, uint8_t type, uint16_t time_cost, uint16_t price,
       bool transfer, uint8_t slot = 0, uint16_t interval_begin = 0,
       uint16_t interval_end = 0)
      : from_(from), to_(to) {
    m_.type_ = type;
    m_.foot_edge_.time_cost_ = time_cost;
    m_.foot_edge_.price_ = price;
    m_.foot_edge_.transfer_ = transfer;
    m_.foot_edge_.slot_ = slot;
    m_.foot_edge_.interval_begin_ = interval_begin;
    m_.foot_edge_.interval_end_ = interval_end;

    assert(m_.type_ != ROUTE_EDGE);
  }

  /** hotel edge constructor. */
  edge(node* station_node, uint16_t checkout_time, uint16_t min_stay_duration,
       uint16_t price, uint16_t slot)
      : from_(station_node), to_(station_node) {
    m_.type_ = HOTEL_EDGE;
    m_.hotel_edge_.checkout_time_ = checkout_time;
    m_.hotel_edge_.min_stay_duration_ = min_stay_duration;
    m_.hotel_edge_.price_ = price;
    m_.hotel_edge_.slot_ = slot;
  }

  edge_cost get_edge_cost(time start_time,
                          light_connection const* last_con) const {
    switch (m_.type_) {
      case ROUTE_EDGE: return get_route_edge_cost(start_time);

      case AFTER_TRAIN_FOOT_EDGE:
        if (last_con == nullptr) {
          return NO_EDGE;
        }
      /* no break */
      case MUMO_EDGE:
      case FOOT_EDGE:
        return edge_cost(m_.foot_edge_.time_cost_, m_.foot_edge_.transfer_,
                         m_.foot_edge_.price_, m_.foot_edge_.slot_);

      case PERIODIC_MUMO_EDGE:
        return edge_cost(calc_time_off(m_.foot_edge_.interval_begin_,
                                       m_.foot_edge_.interval_end_,
                                       start_time % MINUTES_A_DAY) +
                             m_.foot_edge_.time_cost_,
                         m_.foot_edge_.transfer_, m_.foot_edge_.price_,
                         m_.foot_edge_.slot_);
      case TIME_DEPENDENT_MUMO_EDGE:
        return calc_cost_time_dependent_edge(start_time);
      case HOTEL_EDGE: return calc_cost_hotel_edge(start_time);

      case THROUGH_EDGE: return edge_cost(0, false, 0, 0);

      default: return NO_EDGE;
    }
  }

  edge_cost get_minimum_cost() const {
    if (m_.type_ == ROUTE_EDGE) {
      if (m_.route_edge_.conns_.size() == 0) {
        return NO_EDGE;
      } else {
        return edge_cost(
            std::min_element(
                std::begin(m_.route_edge_.conns_),
                std::end(m_.route_edge_.conns_),
                [](light_connection const& c1, light_connection const& c2) {
                  return c1.travel_time() < c2.travel_time();
                })
                ->travel_time(),
            false, std::begin(m_.route_edge_.conns_)->full_con_->price_);
      }
    } else if (m_.type_ == FOOT_EDGE || m_.type_ == AFTER_TRAIN_FOOT_EDGE) {
      return edge_cost(0, m_.foot_edge_.transfer_);
    } else if (m_.type_ == HOTEL_EDGE) {
      return edge_cost(0, false, m_.hotel_edge_.price_);
    } else if (m_.type_ == MUMO_EDGE || m_.type_ == TIME_DEPENDENT_MUMO_EDGE) {
      return edge_cost(0, false, m_.foot_edge_.price_);
    } else {
      return edge_cost(0);
    }
  }

  light_connection const* get_connection(time const start_time) const {
    if (m_.route_edge_.conns_.size() == 0) {
      return nullptr;
    }

    auto it = std::lower_bound(std::begin(m_.route_edge_.conns_),
                               std::end(m_.route_edge_.conns_),
                               light_connection(start_time));

    return (it == std::end(m_.route_edge_.conns_)) ? nullptr : &*it;
  }

  light_connection const* get_connection_reverse(time const start_time) const {
    if (m_.route_edge_.conns_.size() == 0) {
      return nullptr;
    }

    auto it = std::lower_bound(
        m_.route_edge_.conns_.rbegin(), m_.route_edge_.conns_.rend(),
        light_connection(0, start_time, nullptr),
        [](light_connection const& lhs, light_connection const& rhs) {
          return lhs.a_time_ > rhs.a_time_;
        });

    return (it == m_.route_edge_.conns_.rend()) ? nullptr : &*it;
  }

  light_connection const* get_connection(time const start_time) {
    return static_cast<const edge*>(this)->get_connection(start_time);
  }

  light_connection const* get_connection_reverse(time const start_time) {
    return static_cast<const edge*>(this)->get_connection_reverse(start_time);
  }

  edge_cost get_route_edge_cost(time const start_time) const {
    light_connection const* c = get_connection(start_time);
    return (c == nullptr) ? NO_EDGE : edge_cost(c->a_time_ - start_time, c);
  }

  inline node const* get_destination() const { return to_; }
  inline node* get_destination() { return to_; }

  inline bool valid() const { return type() != INVALID_EDGE; }

  inline uint8_t type() const { return m_.type_; }

  inline char const* type_str() const {
    switch (type()) {
      case ROUTE_EDGE: return "ROUTE_EDGE";
      case FOOT_EDGE: return "FOOT_EDGE";
      case AFTER_TRAIN_FOOT_EDGE: return "AFTER_TRAIN_FOOT_EDGE";
      case MUMO_EDGE: return "MUMO_EDGE";
      case TIME_DEPENDENT_MUMO_EDGE: return "TIME_DEPENDENT_MUMO_EDGE";
      case PERIODIC_MUMO_EDGE: return "PERIODIC_MUMO_EDGE";
      case HOTEL_EDGE: return "HOTEL_EDGE";
      case THROUGH_EDGE: return "THROUGH_EDGE";
      default: return "INVALID";
    }
  }

  inline bool empty() const {
    return (type() != ROUTE_EDGE) ? true : m_.route_edge_.conns_.empty();
  }

  uint8_t get_slot() const {
    switch (m_.type_) {
      case MUMO_EDGE:
      case PERIODIC_MUMO_EDGE:
      case TIME_DEPENDENT_MUMO_EDGE: return m_.foot_edge_.slot_;
      case HOTEL_EDGE: return m_.hotel_edge_.slot_;
      default: return 0;
    }
  }

  static time calc_time_off(time const period_begin, time const period_end,
                            time const timestamp) {
    assert(period_begin < MINUTES_A_DAY);
    assert(period_end < MINUTES_A_DAY);
    assert(timestamp < MINUTES_A_DAY);

    /* validity-period begins and ends at the same day */
    if (period_begin <= period_end) {
      if (timestamp < period_begin) {
        return period_begin - timestamp;
      } else if (timestamp > period_end) {
        return (MINUTES_A_DAY - timestamp) + period_begin;
      }
    }
    /* validity-period is over midnight */
    else if (timestamp > period_end && timestamp < period_begin) {
      return period_begin - timestamp;
    }

    return 0;
  }

  node* from_;
  node* to_;

  union edge_details {
    edge_details() { std::memset(this, 0, sizeof(*this)); }

    edge_details(edge_details&& other) noexcept {
      type_ = other.type_;
      if (type_ == ROUTE_EDGE) {
        route_edge_.init_empty();
        route_edge_ = std::move(other.route_edge_);
      } else if (type_ == HOTEL_EDGE) {
        hotel_edge_ = std::move(other.hotel_edge_);
      } else {
        foot_edge_ = std::move(other.foot_edge_);
      }
    }

    edge_details(edge_details const& other) {
      type_ = other.type_;
      if (type_ == ROUTE_EDGE) {
        route_edge_.init_empty();
        route_edge_ = other.route_edge_;
      } else if (type_ == HOTEL_EDGE) {
        hotel_edge_ = std::move(other.hotel_edge_);
      } else {
        foot_edge_.init_empty();
        foot_edge_ = other.foot_edge_;
      }
    }

    edge_details& operator=(edge_details&& other) noexcept {
      type_ = other.type_;
      if (type_ == ROUTE_EDGE) {
        route_edge_.init_empty();
        route_edge_ = std::move(other.route_edge_);
      } else if (type_ == HOTEL_EDGE) {
        hotel_edge_ = std::move(other.hotel_edge_);
      } else {
        foot_edge_.init_empty();
        foot_edge_ = std::move(other.foot_edge_);
      }

      return *this;
    }

    edge_details& operator=(edge_details const& other) {
      type_ = other.type_;
      if (type_ == ROUTE_EDGE) {
        route_edge_.init_empty();
        route_edge_ = other.route_edge_;
      } else if (type_ == HOTEL_EDGE) {
        hotel_edge_ = std::move(other.hotel_edge_);
      } else {
        foot_edge_.init_empty();
        foot_edge_ = other.foot_edge_;
      }

      return *this;
    }

    ~edge_details() {
      if (type_ == ROUTE_EDGE) {
        route_edge_.conns_.~array<light_connection>();
      }
    }

    // placeholder
    uint8_t type_;

    // TYPE = ROUTE_EDGE
    struct {
      uint8_t type_padding_;
      array<light_connection> conns_;

      void init_empty() { new (&conns_) array<light_connection>(); }
    } route_edge_;

    // TYPE = FOOT_EDGE & CO
    struct {
      uint8_t type_padding_;

      // edge weight
      uint16_t time_cost_;
      uint16_t price_;
      bool transfer_;

      // slot for mumo edge
      uint8_t slot_;

      // interval: time-dependent/periodic mumo edge
      uint16_t interval_begin_;
      uint16_t interval_end_;

      void init_empty() {
        time_cost_ = 0;
        price_ = 0;
        transfer_ = false;
        slot_ = 0;
      }
    } foot_edge_;

    // TYPE = HOTEL_EDGE
    struct {
      uint8_t type_padding_;
      uint16_t checkout_time_;
      uint16_t min_stay_duration_;
      uint16_t price_;
      uint8_t slot_;
    } hotel_edge_;
  } m_;

private:
  edge_cost calc_cost_time_dependent_edge(time const start_time) const {
    if (start_time > m_.foot_edge_.interval_end_) {
      return NO_EDGE;
    }
    auto const time_off =
        std::max(0, m_.foot_edge_.interval_begin_ - start_time);
    return edge_cost(time_off + m_.foot_edge_.time_cost_,
                     m_.foot_edge_.transfer_, m_.foot_edge_.price_,
                     m_.foot_edge_.slot_);
  }

  edge_cost calc_cost_hotel_edge(time const start_time) const {
    uint16_t const offset =
        start_time % MINUTES_A_DAY < m_.hotel_edge_.checkout_time_
            ? 0
            : MINUTES_A_DAY;
    auto const duration = std::max(
        m_.hotel_edge_.min_stay_duration_,
        static_cast<uint16_t>((m_.hotel_edge_.checkout_time_ + offset) -
                              (start_time % MINUTES_A_DAY)));
    return edge_cost(duration, false, m_.hotel_edge_.price_,
                     m_.hotel_edge_.slot_);
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
                                          uint16_t time_cost, uint16_t price,
                                          uint8_t slot, uint16_t interval_begin,
                                          uint16_t interval_end) {
  return edge(from, to, edge::TIME_DEPENDENT_MUMO_EDGE, time_cost, price, false,
              slot, interval_begin, interval_end);
}

inline edge make_periodic_mumo_edge(node* from, node* to, uint16_t time_cost,
                                    uint16_t price, uint8_t slot,
                                    uint16_t interval_begin,
                                    uint16_t interval_end) {
  return edge(from, to, edge::PERIODIC_MUMO_EDGE, time_cost, price, false, slot,
              interval_begin, interval_end);
}

inline edge make_hotel_edge(node* station_node, uint16_t checkout_time,
                            uint16_t min_stay_duration, uint16_t price,
                            uint8_t slot) {
  return edge(station_node, checkout_time, min_stay_duration, price, slot);
}

inline edge make_invalid_edge(node* from, node* to) {
  return edge(from, to, edge::INVALID_EDGE, 0, 0, false, 0);
}

inline edge make_through_edge(node* from, node* to) {
  return edge(from, to, edge::THROUGH_EDGE, 0, 0, false, 0);
}

}  // namespace motis
