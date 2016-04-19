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
          a_platform = last_con->full_con_->a_platform_;
          a_time = last_con->a_time_;
        }

        walk_arrival = INVALID_TIME;

		auto s1 = std::next(it, 1) == end(labels) ? nullptr : *std::next(it, 1);
		auto s2 = s1 == nullptr || std::next(it, 2) == end(labels) ? nullptr : *std::next(it, 2);
        if (s2 && s2->connection_ != nullptr) {
          d_platform = s2->connection_->full_con_->d_platform_;
          d_time = s2->connection_->d_time_;
        }

        stops.emplace_back(static_cast<unsigned int>(++station_index),
                           current->node_->get_station()->id_, a_platform,
                           d_platform, a_time, d_time,
                           a_time != INVALID_TIME && d_time != INVALID_TIME &&
                               last_con != nullptr);
        break;
      }

      case WALK:
        assert(std::next(it) != end(labels));

        stops.emplace_back(
            static_cast<unsigned int>(++station_index),
            current->node_->get_station()->id_,
            last_con == nullptr ? MOTIS_UNKNOWN_TRACK
                                : last_con->full_con_->a_platform_,
            MOTIS_UNKNOWN_TRACK,
            stops.empty() ? INVALID_TIME : (last_con == nullptr)
                                               ? current->now_
                                               : last_con->a_time_,
            current->now_, last_con != nullptr);

        transports.emplace_back(station_index,
                                static_cast<unsigned int>(station_index) + 1,
                                (*std::next(it))->now_ - current->now_, -1, 0);

        walk_arrival = (*std::next(it))->now_;

        last_con = nullptr;
        break;

      case IN_CONNECTION: {
        if (current->connection_) {
          transports.emplace_back(static_cast<unsigned int>(station_index),
                                  static_cast<unsigned int>(station_index) + 1,
                                  current->connection_, current->node_->route_);
        }

        // do not collect the last connection route node.
        assert(std::next(it) != end(labels));
        auto succ = *std::next(it);

        if (succ->node_->is_route_node()) {
          // skip through edge.
          if (!succ->connection_) {
            succ = *std::next(it, 2);
          }

          stops.emplace_back(static_cast<unsigned int>(++station_index),
                             current->node_->get_station()->id_,
                             current->connection_->full_con_->a_platform_,
                             succ->connection_->full_con_->d_platform_,
                             current->connection_->a_time_,
                             succ->connection_->d_time_, false);
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
