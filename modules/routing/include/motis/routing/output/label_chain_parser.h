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
      if (n && n->get_node()->is_station_node()) {
        return WALK;
      } else {
        return PRE_CONNECTION;
      }
    case PRE_CONNECTION: return IN_CONNECTION;
    case IN_CONNECTION_THROUGH: return IN_CONNECTION;
    case IN_CONNECTION_THROUGH_SKIP:
      return c->get_node()->is_foot_node() ? WALK : AT_STATION;
    case IN_CONNECTION:
      if (c->connection_ == nullptr) {
        switch (c->get_node()->type()) {
          case node_type::STATION_NODE:
            return n && n->get_node()->is_station_node() ? WALK : AT_STATION;
          case node_type::FOOT_NODE: return WALK;
          case node_type::ROUTE_NODE:
            return n->get_node()->is_route_node() ? IN_CONNECTION_THROUGH
                                                  : IN_CONNECTION_THROUGH_SKIP;
        }
      } else {
        return IN_CONNECTION;
      }
    case WALK: return n && n->get_node()->is_station_node() ? WALK : AT_STATION;
  }
  return static_cast<state>(s);
};

template <typename LabelIt>
int initial_state(LabelIt& it) {
  if ((*std::next(it))->get_node()->is_station_node()) {
    return WALK;
  } else if ((*std::next(it))->get_node()->is_foot_node()) {
    ++it;
    return WALK;
  } else {
    return AT_STATION;
  }
}

template <typename Label>
std::pair<std::vector<intermediate::stop>, std::vector<intermediate::transport>>
parse_label_chain(schedule const& sched, Label* terminal_label,
                  search_dir const dir) {
  std::vector<Label*> labels;

  auto c = terminal_label;
  do {
    labels.insert(dir == search_dir::FWD ? begin(labels) : end(labels), c);
  } while ((c = c->pred_));

  edge first_edge;
  if (dir == search_dir::BWD) {
    for (unsigned i = 1; i < labels.size(); ++i) {
      labels[i]->edge_ = labels[i - 1]->edge_;
    }

    auto const second_edge = labels[0]->edge_;
    first_edge = make_invalid_edge(nullptr, second_edge->from_);
    labels[0]->edge_ = &first_edge;
  }

  std::pair<std::vector<intermediate::stop>,
            std::vector<intermediate::transport>>
      ret;
  auto& stops = ret.first;
  auto& transports = ret.second;

  node const* last_route_node = nullptr;
  light_connection const* last_con = nullptr;
  auto walk_arrival = INVALID_TIME;
  auto walk_arrival_di = delay_info({nullptr, INVALID_TIME, event_type::DEP});
  auto stop_index = -1;

  auto it = begin(labels);
  int current_state = initial_state(it);
  while (it != end(labels)) {
    auto current = *it;

    switch (current_state) {
      case AT_STATION: {
        if (current->edge_->type() == edge::HOTEL_EDGE &&
            (*std::next(it))->get_node()->is_foot_node()) {
          break;
        }
        int a_track = MOTIS_UNKNOWN_TRACK;
        int d_track = MOTIS_UNKNOWN_TRACK;
        time a_time = walk_arrival, a_sched_time = walk_arrival;
        time d_time = INVALID_TIME, d_sched_time = INVALID_TIME;
        timestamp_reason a_reason = walk_arrival_di.get_reason(),
                         d_reason = timestamp_reason::SCHEDULE;
        if (a_time == INVALID_TIME && last_con != nullptr) {
          a_track = last_con->full_con_->a_track_;
          a_time = last_con->a_time_;

          auto a_di =
              get_delay_info(sched, last_route_node, last_con, event_type::ARR);
          a_sched_time = a_di.get_schedule_time();
          a_reason = a_di.get_reason();
        }

        walk_arrival = INVALID_TIME;

        auto s1 = std::next(it, 1);
        auto s2 = std::next(it, 2);
        if (s1 != end(labels) && s2 != end(labels) &&
            (*s2)->connection_ != nullptr) {
          auto const& succ = *s2;
          d_track = succ->connection_->full_con_->d_track_;
          d_time = succ->connection_->d_time_;

          auto d_di = get_delay_info(sched, (*s1)->get_node(),
                                     succ->connection_, event_type::DEP);
          d_sched_time = d_di.get_schedule_time();
          d_reason = d_di.get_reason();
        }

        stops.emplace_back(static_cast<unsigned int>(++stop_index),
                           current->get_node()->get_station()->id_, a_track,
                           d_track, a_time, d_time, a_sched_time, d_sched_time,
                           a_reason, d_reason,
                           a_time != INVALID_TIME && d_time != INVALID_TIME &&
                               last_con != nullptr);
        break;
      }

      case WALK:
        assert(std::next(it) != end(labels));

        if (last_con != nullptr) {
          walk_arrival_di =
              get_delay_info(sched, last_route_node, last_con, event_type::ARR);
        }

        stops.emplace_back(
            static_cast<unsigned int>(++stop_index),
            current->get_node()->get_station()->id_,
            last_con == nullptr ? MOTIS_UNKNOWN_TRACK
                                : last_con->full_con_->a_track_,
            MOTIS_UNKNOWN_TRACK,

            // Arrival graph time:
            stops.empty() ? INVALID_TIME : last_con ? last_con->a_time_
                                                    : current->now_,

            // Departure graph time:
            current->now_,

            // Arrival schedule time:
            stops.empty() ? INVALID_TIME
                          : last_con ? walk_arrival_di.get_schedule_time()
                                     : current->now_,

            // Departure schedule time:
            current->now_,

            walk_arrival_di.get_reason(), walk_arrival_di.get_reason(),

            last_con != nullptr);

        transports.emplace_back(stop_index,
                                static_cast<unsigned int>(stop_index) + 1,
                                (*std::next(it))->now_ - current->now_,
                                (*std::next(it))->edge_->get_mumo_id(), 0);

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

        if (succ->get_node()->is_route_node()) {
          auto dep_route_node = current->get_node();

          // skip through edge.
          if (!succ->connection_) {
            dep_route_node = succ->get_node();
            succ = *std::next(it, 2);
          }

          // through edge used but not the route edge after that
          // (instead: went to station node using the leaving edge)
          if (succ->connection_) {
            auto a_di = get_delay_info(sched, current->get_node(),
                                       current->connection_, event_type::ARR);
            auto d_di = get_delay_info(sched, dep_route_node, succ->connection_,
                                       event_type::DEP);

            stops.emplace_back(
                static_cast<unsigned int>(++stop_index),
                current->get_node()->get_station()->id_,
                current->connection_->full_con_->a_track_,
                succ->connection_->full_con_->d_track_,
                current->connection_->a_time_, succ->connection_->d_time_,
                a_di.get_schedule_time(), d_di.get_schedule_time(),
                a_di.get_reason(), d_di.get_reason(), false);
          }
        }

        last_route_node = current->get_node();
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
