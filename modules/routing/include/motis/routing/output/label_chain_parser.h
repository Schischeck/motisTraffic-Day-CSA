#pragma once

#include <utility>
#include <vector>

#include "motis/core/schedule/connection.h"
#include "motis/core/access/realtime_access.h"

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
  IN_CONNECTION_THROUGH,
  IN_CONNECTION_THROUGH_SKIP
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
    case IN_CONNECTION_THROUGH: return IN_CONNECTION;
    case IN_CONNECTION_THROUGH_SKIP:
      return c->node_->is_foot_node() ? WALK : AT_STATION;
    case IN_CONNECTION:
      if (c->connection_ == nullptr) {
        switch (c->node_->type()) {
          case node_type::STATION_NODE: return AT_STATION;
          case node_type::FOOT_NODE: return WALK;
          case node_type::ROUTE_NODE:
            return n->node_->is_route_node() ? IN_CONNECTION_THROUGH
                                             : IN_CONNECTION_THROUGH_SKIP;
        }
      } else {
        return IN_CONNECTION;
      }
    case WALK:
      return n != nullptr && n->node_->is_station_node() ? WALK : AT_STATION;
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
parse_label_chain(schedule const& sched, Label const* terminal_label) {
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

  node const* last_route_node = nullptr;
  light_connection const* last_con = nullptr;
  time walk_arrival = INVALID_TIME;
  int stop_index = -1;

  auto it = begin(labels);
  int current_state = initial_state(it);
  while (it != end(labels)) {
    auto current = *it;

    switch (current_state) {
      case AT_STATION: {
        int a_platform = MOTIS_UNKNOWN_TRACK;
        int d_platform = MOTIS_UNKNOWN_TRACK;
        time a_time = walk_arrival, a_sched_time = walk_arrival;
        time d_time = INVALID_TIME, d_sched_time = INVALID_TIME;
        if (a_time == INVALID_TIME && last_con != nullptr) {
          a_platform = last_con->full_con_->a_platform_;
          a_time = last_con->a_time_;
          a_sched_time = get_schedule_time(sched, last_route_node, last_con,
                                           event_type::ARR);
        }

        walk_arrival = INVALID_TIME;

        auto s1 = std::next(it, 1);
        auto s2 = std::next(it, 2);
        if (s1 != end(labels) && s2 != end(labels) &&
            (*s2)->connection_ != nullptr) {
          auto const& succ = *s2;
          d_platform = succ->connection_->full_con_->d_platform_;
          d_time = succ->connection_->d_time_;
          d_sched_time = get_schedule_time(sched, (*s1)->node_,
                                           succ->connection_, event_type::DEP);
        }

        stops.emplace_back(static_cast<unsigned int>(++stop_index),
                           current->node_->get_station()->id_, a_platform,
                           d_platform, a_time, d_time, a_sched_time,
                           d_sched_time,
                           a_time != INVALID_TIME && d_time != INVALID_TIME &&
                               last_con != nullptr);
        break;
      }

      case WALK:
        assert(std::next(it) != end(labels));

        stops.emplace_back(
            static_cast<unsigned int>(++stop_index),
            current->node_->get_station()->id_,
            last_con == nullptr ? MOTIS_UNKNOWN_TRACK
                                : last_con->full_con_->a_platform_,
            MOTIS_UNKNOWN_TRACK,

            // Arrival graph time:
            stops.empty() ? INVALID_TIME : last_con ? last_con->a_time_
                                                    : current->now_,

            // Departure graph time:
            current->now_,

            // Arrival schedule time:
            stops.empty()
                ? INVALID_TIME
                : last_con ? get_schedule_time(sched, last_route_node, last_con,
                                               event_type::ARR)
                           : current->now_,

            // Departure schedule time:
            current->now_,

            last_con != nullptr);

        transports.emplace_back(stop_index,
                                static_cast<unsigned int>(stop_index) + 1,
                                (*std::next(it))->now_ - current->now_, -1, 0);

        walk_arrival = (*std::next(it))->now_;

        last_con = nullptr;
        break;

      case IN_CONNECTION: {
        if (current->connection_) {
          transports.emplace_back(static_cast<unsigned int>(stop_index),
                                  static_cast<unsigned int>(stop_index) + 1,
                                  current->connection_);
        }

        // do not collect the last connection route node.
        assert(std::next(it) != end(labels));
        auto succ = *std::next(it);

        if (succ->node_->is_route_node()) {
          auto dep_route_node = current->node_;

          // skip through edge.
          if (!succ->connection_) {
            dep_route_node = succ->node_;
            succ = *std::next(it, 2);
          }

          // through edge used but not the route edge after that
          // (instead: went to station node using the leaving edge)
          if (succ->connection_) {
            stops.emplace_back(
                static_cast<unsigned int>(++stop_index),
                current->node_->get_station()->id_,
                current->connection_->full_con_->a_platform_,
                succ->connection_->full_con_->d_platform_,
                current->connection_->a_time_, succ->connection_->d_time_,
                get_schedule_time(sched, current->node_, current->connection_,
                                  event_type::ARR),
                get_schedule_time(sched, dep_route_node, succ->connection_,
                                  event_type::DEP),
                false);
          }
        }

        last_route_node = current->node_;
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