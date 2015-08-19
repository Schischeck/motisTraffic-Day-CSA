#pragma once

#include <cmath>
#include <cstring>
#include <cmath>
#include <memory>

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/connection.h"
#include "motis/routing/lower_bounds.h"
#include "motis/routing/filters.h"
#include "motis/routing/memory_manager.h"

#define MINUTELY_WAGE 8
#define TRANSFER_WAGE 0
#define MAX_LABELS 100000000
#define MAX_LABELS_WITH_MARGIN (MAX_LABELS + 1000)
#define MOTIS_MAX_REGIONAL_TRAIN_TICKET_PRICE (4200u)

namespace motis {

class label {
public:
  enum {
    MOTIS_LOCAL_TRANSPORT_PRICE,
    MOTIS_REGIONAL_TRAIN_PRICE,
    MOTIS_IC_PRICE,
    MOTIS_ICE_PRICE,
    ADDITIONAL_PRICE
  };

  label() = default;

  label(node const* node, label* pred, time now, lower_bounds& lower_bounds)
      : _pred(pred),
        _node(node),
        _connection(nullptr),
        _start(pred != nullptr ? pred->_start : now),
        _now(now),
        _dominated(false),
        _target_slot(pred == nullptr ? 0 : pred->_target_slot) {
    auto transfers_lb = lower_bounds.transfers.get_distance(node->_id);
    if (transfers_lb == std::numeric_limits<uint32_t>::max()) {
      _travel_time[1] = std::numeric_limits<uint16_t>::max();
      return;
    }

    _travel_time[0] = _now - _start;
    _travel_time[1] = _travel_time[0];
    _travel_time[1] += lower_bounds.travel_time.get_distance(node->_id);

    _transfers[0] = pred != nullptr ? pred->_transfers[0] : 0;
    _transfers[1] = _transfers[0];
    _transfers[1] += transfers_lb;

    _total_price[0] = pred != nullptr ? pred->_total_price[0] : 0;

    if (pred != nullptr)
      std::memcpy(_prices, pred->_prices, sizeof(_prices));
    else
      std::memset(_prices, 0, sizeof(_prices));

    set_total_price_lower_bound(lower_bounds.price.get_distance(node->_id));
  }

  label* create_label(edge const& edge, lower_bounds& lower_bounds,
                      memory_manager<label>& label_store) {
    auto n_node = edge.get_destination()->_id;

    uint32_t transfers_l_b = lower_bounds.transfers.get_distance(n_node);
    if (transfers_l_b == std::numeric_limits<uint32_t>::max()) return nullptr;

    edge_cost ec = edge.get_edge_cost(_now, _connection);

    unsigned n_travel_time = _travel_time[0] + ec.time;
    unsigned n_travel_time_l_b = n_travel_time;
    n_travel_time_l_b += lower_bounds.travel_time.get_distance(n_node);

    unsigned n_transfers = _transfers[0] + (ec.transfer ? 1 : 0);
    unsigned n_transfers_l_b = n_transfers + transfers_l_b;

    if (ec == NO_EDGE ||
        (_pred != nullptr && edge.get_destination() == _pred->_node) ||
        is_filtered_travel_time(n_travel_time_l_b) ||
        is_filtered_transfers(n_transfers_l_b) ||
        is_filtered_waiting_time(ec.connection, _travel_time[0], n_travel_time))
      return nullptr;

    label* l = new (label_store.create()) label(*this);
    l->_travel_time[0] = n_travel_time;
    l->_travel_time[1] = n_travel_time_l_b;
    l->_transfers[0] = n_transfers;
    l->_transfers[1] = n_transfers_l_b;
    l->add_price_of_connection(ec, edge._m._type == edge::MUMO_EDGE);
    l->set_total_price_lower_bound(lower_bounds.price.get_distance(n_node));
    l->_pred = this;
    l->_start = _start;
    l->_now = _now + ec.time;
    l->_node = edge.get_destination();
    l->_connection = ec.connection;

    return l;
  }

  bool dominates(label const& o, bool lower_bound) const {
    /* --- CHECK MAY DOMINATE --- */
    if (_start < o._start || _now > o._now) return false;

    int index = lower_bound ? 1 : 0;
    bool could_dominate = false;

    /* --- TRANSFERS --- */
    if (_transfers[index] > o._transfers[index]) return false;
    could_dominate = could_dominate || _transfers[index] < o._transfers[index];

    /* --- TRAVEL TIME --- */
    if (_travel_time[index] > o._travel_time[index]) return false;
    could_dominate =
        could_dominate || _travel_time[index] < o._travel_time[index];

    /* --- PRICE --- */
    unsigned my_price = _total_price[index];
    unsigned o_price = o._total_price[index];

    if (lower_bound) {
      my_price +=
          _travel_time[1] * MINUTELY_WAGE + _transfers[1] * TRANSFER_WAGE;
      o_price +=
          o._travel_time[1] * MINUTELY_WAGE + o._transfers[1] * TRANSFER_WAGE;
    } else {
      my_price +=
          (o._now - _start) * MINUTELY_WAGE + _transfers[0] * TRANSFER_WAGE;
      o_price +=
          (o._now - o._start) * MINUTELY_WAGE + o._transfers[0] * TRANSFER_WAGE;
    }

#ifdef PRICE_TOLERANCE
    unsigned tolerance = std::floor(0.01 * std::min(my_price, o_price));
    if (std::labs(my_price - o_price) > tolerance) {
#endif
      if (my_price > o_price) return false;

      could_dominate = could_dominate || my_price < o_price;
#ifdef PRICE_TOLERANCE
    }
#endif

    /* --- ALL CRITERIA --- */
    return true;
  }

  bool dominates_hard(label const& o) const {
    bool could_dominate = false;

    /* --- TRANSFERS --- */
    if (_transfers[0] > o._transfers[0]) return false;
    could_dominate = could_dominate || _transfers[0] < o._transfers[0];

    /* --- TRAVEL TIME --- */
    if (_travel_time[0] > o._travel_time[0]) return false;
    could_dominate = could_dominate || _travel_time[0] < o._travel_time[0];

    /* --- PRICE --- */
    unsigned my_price = get_price_with_wages(false);
    unsigned o_price = o.get_price_with_wages(false);
    if (my_price > o_price) return false;
    could_dominate = could_dominate || my_price < o_price;

    return could_dominate || _start >= o._start;
  }

  bool operator<(label const& o) const {
    if (_travel_time[1] != o._travel_time[1])
      return _travel_time[1] < o._travel_time[1];

    if (_transfers[1] != o._transfers[1])
      return _transfers[1] < o._transfers[1];

    unsigned my_price_heuristic = get_price_with_wages(true);
    unsigned o_price_heuristic = o.get_price_with_wages(true);
    return my_price_heuristic < o_price_heuristic || _start > o._start;
  }

  unsigned get_travel_time_price() const {
    return _travel_time[0] * MINUTELY_WAGE;
  }

  int get_price_with_wages(bool lower_bound) const {
    return _total_price[lower_bound ? 1 : 0] +
           _travel_time[lower_bound ? 1 : 0] * MINUTELY_WAGE +
           _transfers[lower_bound ? 1 : 0] * TRANSFER_WAGE;
  }

  void add_price_of_connection(edge_cost const& ec, bool mumo_edge) {
    if (ec.connection != nullptr) {
      connection const* con = ec.connection->_full_con;
      switch (con->clasz) {
        case MOTIS_ICE:
          if (_prices[MOTIS_ICE_PRICE] == 0) {
            if (_prices[MOTIS_IC_PRICE] != 0)
              _prices[MOTIS_ICE_PRICE] += 100 + con->price;
            else
              _prices[MOTIS_ICE_PRICE] += 700 + con->price;
          } else
            _prices[MOTIS_ICE_PRICE] += con->price;

          break;

        case MOTIS_IC:
          if (_prices[MOTIS_IC_PRICE] == 0) {
            if (_prices[MOTIS_ICE_PRICE] != 0)
              _prices[MOTIS_IC_PRICE] += con->price;
            else
              _prices[MOTIS_IC_PRICE] += 600 + con->price;
          } else
            _prices[MOTIS_IC_PRICE] += 600 + con->price;

          break;

        case MOTIS_RE:
        case MOTIS_RB:
        case MOTIS_S:
          _prices[MOTIS_REGIONAL_TRAIN_PRICE] += con->price;
          if (_prices[MOTIS_REGIONAL_TRAIN_PRICE] >
              MOTIS_MAX_REGIONAL_TRAIN_TICKET_PRICE)
            _prices[MOTIS_REGIONAL_TRAIN_PRICE] =
                MOTIS_MAX_REGIONAL_TRAIN_TICKET_PRICE;

          break;

        case MOTIS_N:
        case MOTIS_U:
        case MOTIS_STR:
        case MOTIS_BUS:
        case MOTIS_X: _prices[MOTIS_LOCAL_TRANSPORT_PRICE] += con->price;
      }
    } else {
      _prices[4] += ec[2];
      if (mumo_edge) set_slot(false, ec.slot);
    }

    unsigned price_sum = _prices[0] + _prices[1] + _prices[2] + _prices[3];
    auto train_price = std::min(price_sum, 14000u);

    _total_price[0] = train_price + _prices[4];
  }

  void set_total_price_lower_bound(uint32_t lower_bound) {
    assert(lower_bound <= std::numeric_limits<uint16_t>::max());
    _total_price[1] =
        _prices[ADDITIONAL_PRICE] +
        std::min(14000u,
                 _prices[MOTIS_LOCAL_TRANSPORT_PRICE] +
                     _prices[MOTIS_IC_PRICE] + _prices[MOTIS_ICE_PRICE] +
                     std::min(_prices[MOTIS_REGIONAL_TRAIN_PRICE] + lower_bound,
                              MOTIS_MAX_REGIONAL_TRAIN_TICKET_PRICE));
  }

  void set_slot(bool start, int slot) {
    if (!start)
      _target_slot = slot;
    else if (_pred == nullptr)
      _travel_time[0] = slot;
    else
      _pred->set_slot(start, slot);
  }

  int get_slot(bool start) const {
    if (!start)
      return _target_slot;
    else {
      label const* current = this;
      label const* pred = _pred;
      while (pred != nullptr) {
        auto old_pred = pred;
        pred = current->_pred;
        current = old_pred;
      }
      return current->_travel_time[0];
    }
  }

  label* _pred;
  node const* _node;
  light_connection const* _connection;
  uint16_t _travel_time[2];
  uint16_t _total_price[2];
  uint16_t _prices[5];
  time _start, _now;
  uint8_t _transfers[2];
  bool _dominated;
  uint8_t _target_slot;
};

}  // namespace motis
