#pragma once

#include <utility>
#include <vector>

#include "motis/core/schedule/connection.h"

#include "motis/routing/output/stop.h"
#include "motis/routing/output/transport.h"

#define MOTIS_UNKNOWN_TRACK (0)

namespace motis {
namespace routing {
namespace output {

enum state {
  AT_STATION,
  PRE_CONNECTION,
  IN_CONNECTION,
  PRE_WALK,
  WALK,
  IN_CONNECTION_THROUGH
};

template <typename Label>
state next_state(int s, Label const* c, Label const* n) {
  switch (s) {
    case AT_STATION:
      if (n && n->node_->is_station_node()) {
        return WALK;
      } else {
        return PRE_CONNECTION;
      }
    case PRE_CONNECTION: return IN_CONNECTION;
    case IN_CONNECTION:
      if (c->connection_ == nullptr) {
        if (n->node_->is_station_node()) {
          return WALK;
        } else if (c->node_->is_route_node()) {
          return IN_CONNECTION_THROUGH;
        } else {
          return AT_STATION;
        }
      } else {
        return IN_CONNECTION;
      }
    case IN_CONNECTION_THROUGH: return IN_CONNECTION;
    case WALK: return AT_STATION;
  }
  return static_cast<state>(s);
};

template <typename LabelIt>
int initial_state(LabelIt& it) {
  if ((*std::next(it))->node_->is_station_node()) {
    return WALK;
  } else if ((*std::next(it))->node_->is_foot_node()) {
    ++it;
    return WALK;
  } else {
    return AT_STATION;
  }
}

template <typename Label>
std::pair<std::vector<intermediate::stop>, std::vector<intermediate::transport>>
parse_label_chain(Label const* terminal_label) {
  std::vector<Label const*> labels;

  auto c = terminal_label;
  do {
    labels.insert(begin(labels), c);
  } while ((c = c->pred_));

  std::pair<std::vector<intermediate::stop>,
            std::vector<intermediate::transport>>
      ret;
  auto& stops = ret.first;
  auto& transports = ret.second;

  light_connection const* last_con = nullptr;
  time walk_arrival = INVALID_TIME;
  int station_index = -1;

  auto it = begin(labels);
  int current_state = initial_state(it);
  while (it != end(labels)) {
    auto current = *it;

    switch (current_state) {
      case AT_STATION: {
        int a_platform = MOTIS_UNKNOWN_TRACK;
        int d_platform = MOTIS_UNKNOWN_TRACK;
        time a_time = walk_arrival;
        time d_time = INVALID_TIME;
        if (a_time == INVALID_TIME && last_con != nullptr) {
          a_platform = last_con->_full_con->a_platform;
          a_time = last_con->a_time;
        }

        walk_arrival = INVALID_TIME;

        auto s1 = std::next(it, 1);
        auto s2 = std::next(it, 2);
        if (s1 != end(labels) && s2 != end(labels) &&
            (*s2)->connection_ != nullptr) {
          d_platform = (*s2)->connection_->_full_con->d_platform;
          d_time = (*s2)->connection_->d_time;
        }

        stops.emplace_back((unsigned int)++station_index,
                           current->node_->get_station()->_id, a_platform,
                           d_platform, a_time, d_time,
                           a_time != INVALID_TIME && d_time != INVALID_TIME &&
                               last_con != nullptr);
        break;
      }

      case WALK:
        assert(std::next(it) != end(labels));

        stops.emplace_back(
            (unsigned int)++station_index, current->node_->get_station()->_id,
            last_con == nullptr ? MOTIS_UNKNOWN_TRACK
                                : last_con->_full_con->a_platform,
            MOTIS_UNKNOWN_TRACK,
            stops.empty() ? INVALID_TIME : (last_con == nullptr)
                                               ? current->now_
                                               : last_con->a_time,
            current->now_, last_con != nullptr);

        transports.emplace_back(station_index, (unsigned int)station_index + 1,
                                (*std::next(it))->now_ - current->now_, -1, 0);

        walk_arrival = (*std::next(it))->now_;

        last_con = nullptr;
        break;

      case IN_CONNECTION: {
        if (current->connection_) {
          transports.emplace_back((unsigned int)station_index,
                                  (unsigned int)station_index + 1,
                                  current->connection_, current->node_->_route);
        }

        // do not collect the last connection route node.
        assert(std::next(it) != end(labels));
        auto succ = *std::next(it);

        if (succ->node_->is_route_node()) {
          // skip through edge.
          if (!succ->connection_) {
            succ = *std::next(it, 2);
          }

          stops.emplace_back(
              (unsigned int)++station_index, current->node_->get_station()->_id,
              current->connection_->_full_con->a_platform,
              succ->connection_->_full_con->d_platform,
              current->connection_->a_time, succ->connection_->d_time, false);
        }

        last_con = current->connection_;
        break;
      }
    }

    ++it;
    if (it != end(labels)) {
      current = *it;
      auto next = it == end(labels) || std::next(it) == end(labels)
                      ? nullptr
                      : *std::next(it);
      current_state = next_state(current_state, current, next);
    }
  }

  return ret;
}

}  // namespace output
}  // namespace routing
}  // namespace motis
