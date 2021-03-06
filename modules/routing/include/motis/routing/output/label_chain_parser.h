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
  WALK_SKIP,
  IN_CONNECTION_THROUGH,
  IN_CONNECTION_THROUGH_SKIP
};

template <typename Label>
inline node* get_node(Label const& l) {
  return l.edge_->to_;
}

template <typename Label>
state next_state(int s, Label const* c, Label const* n) {
  switch (s) {
    case AT_STATION:
      if (n && get_node(*n)->is_station_node()) {
        return WALK;
      } else {
        return PRE_CONNECTION;
      }
    case PRE_CONNECTION: return IN_CONNECTION;
    case IN_CONNECTION_THROUGH: return IN_CONNECTION;
    case IN_CONNECTION_THROUGH_SKIP:
      return get_node(*c)->is_foot_node() ? WALK : AT_STATION;
    case IN_CONNECTION:
      if (c->connection_ == nullptr) {
        if (n && get_node(*c)->type() == node_type::STATION_NODE &&
            get_node(*n)->type() == node_type::FOOT_NODE) {
          return WALK_SKIP;
        }

        switch (get_node(*c)->type()) {
          case node_type::STATION_NODE:
            return n && get_node(*n)->is_station_node() ? WALK : AT_STATION;
          case node_type::FOOT_NODE: return WALK;
          case node_type::ROUTE_NODE:
            return get_node(*n)->is_route_node() ? IN_CONNECTION_THROUGH
                                                 : IN_CONNECTION_THROUGH_SKIP;
        }
      } else {
        return IN_CONNECTION;
      }
      break;
    case WALK:
      if (n && get_node(*n)->is_foot_node()) {
        return WALK_SKIP;
      } else if (n && get_node(*n)->is_station_node()) {
        return WALK;
      } else {
        return AT_STATION;
      }
    case WALK_SKIP: return WALK;
  }
  return static_cast<state>(s);
};

template <typename LabelIt>
int initial_state(LabelIt& it) {
  if (get_node(*std::next(it))->is_station_node()) {
    return WALK;
  } else if (get_node(*std::next(it))->is_foot_node()) {
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
  std::vector<Label> labels;

  auto c = terminal_label;
  do {
    labels.insert(begin(labels), *c);
  } while ((c = c->pred_));

  node last_node(sched.station_nodes_.at(0).get(), 0, 0);
  edge last_edge;
  if (dir == search_dir::BWD) {
    for (unsigned i = 0; i < labels.size() - 1; ++i) {
      labels[i].edge_ = labels[i + 1].edge_;
      labels[i].connection_ = labels[i + 1].connection_;
    }

    auto& last_label = labels[labels.size() - 1];
    auto const second_edge = last_label.edge_;
    last_edge = make_invalid_edge(&last_node, second_edge->from_);
    last_label.edge_ = &last_edge;
    last_label.connection_ = nullptr;

    std::reverse(begin(labels), end(labels));
  }

  std::pair<std::vector<intermediate::stop>,
            std::vector<intermediate::transport>>
      ret;
  auto& stops = ret.first;
  auto& transports = ret.second;

  node const* last_route_node = nullptr;
  light_connection const* last_con = nullptr;
  uint16_t last_con_day_idx = 0;
  auto walk_arrival = INVALID_TIME;
  auto walk_arrival_di = delay_info({nullptr, 0, 0, event_type::DEP});
  auto stop_index = -1;

  auto it = begin(labels);
  int current_state = initial_state(it);
  while (it != end(labels)) {
    auto& current = *it;

    switch (current_state) {
      case AT_STATION: {
        if (current.edge_->type() == edge::HOTEL_EDGE &&
            get_node(*std::next(it))->is_foot_node()) {
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
          a_time = last_con->event_time(event_type::ARR, last_con_day_idx);

          auto a_di = get_delay_info(sched, last_route_node, last_con,
                                     last_con_day_idx, event_type::ARR);
          a_sched_time = a_di.get_schedule_time();
          a_reason = a_di.get_reason();
        }

        walk_arrival = INVALID_TIME;

        auto s1 = std::next(it, 1);
        auto s2 = std::next(it, 2);
        if (s1 != end(labels) && s2 != end(labels) &&
            s2->connection_ != nullptr) {
          auto const& succ = *s2;
          d_track = succ.connection_->full_con_->d_track_;
          d_time = succ.connection_->event_time(event_type::DEP, succ.day_idx_);

          auto d_di = get_delay_info(sched, get_node(*s1), succ.connection_,
                                     succ.day_idx_, event_type::DEP);
          d_sched_time = d_di.get_schedule_time();
          d_reason = d_di.get_reason();
        }

        stops.emplace_back(static_cast<unsigned int>(++stop_index),
                           get_node(current)->get_station()->id_, a_track,
                           d_track, a_time, d_time, a_sched_time, d_sched_time,
                           a_reason, d_reason,
                           last_con != nullptr && a_time != INVALID_TIME,
                           d_time != INVALID_TIME);
        break;
      }

      case WALK:
        assert(std::next(it) != end(labels));

        if (last_con != nullptr) {
          walk_arrival_di = get_delay_info(sched, last_route_node, last_con,
                                           last_con_day_idx, event_type::ARR);
        }

        stops.emplace_back(
            static_cast<unsigned int>(++stop_index),
            get_node(current)->get_station()->id_,
            last_con == nullptr ? MOTIS_UNKNOWN_TRACK
                                : last_con->full_con_->a_track_,
            MOTIS_UNKNOWN_TRACK,

            // Arrival graph time:
            stops.empty() ? INVALID_TIME
                          : last_con ? last_con->event_time(event_type::ARR,
                                                            current.day_idx_)
                                     : current.now_,

            // Departure graph time:
            current.now_,

            // Arrival schedule time:
            stops.empty()
                ? INVALID_TIME
                : last_con ? walk_arrival_di.get_schedule_time() : current.now_,

            // Departure schedule time:
            current.now_,

            // Arrival reason timestamp
            walk_arrival_di.get_reason(),

            // Departure reason timestamp
            walk_arrival_di.get_reason(),

            // Leaving
            last_con != nullptr,

            // Entering
            false);

        transports.emplace_back(stop_index,
                                static_cast<unsigned int>(stop_index) + 1,
                                (std::next(it)->now_ - current.now_).ts(),
                                std::next(it)->edge_->get_mumo_id(), 0);

        walk_arrival = std::next(it)->now_;
        last_con = nullptr;
        break;

      case IN_CONNECTION: {
        if (current.connection_) {
          transports.emplace_back(static_cast<unsigned int>(stop_index),
                                  static_cast<unsigned int>(stop_index) + 1,
                                  current.connection_);
        }

        // do not collect the last connection route node.
        assert(std::next(it) != end(labels));
        auto succ = *std::next(it);

        if (get_node(succ)->is_route_node()) {
          auto dep_route_node = get_node(current);

          // skip through edge.
          if (!succ.connection_) {
            dep_route_node = get_node(succ);
            succ = *std::next(it, 2);
          }

          // through edge used but not the route edge after that
          // (instead: went to station node using the leaving edge)
          if (succ.connection_) {
            auto a_di =
                get_delay_info(sched, get_node(current), current.connection_,
                               current.day_idx_, event_type::ARR);
            auto d_di = get_delay_info(sched, dep_route_node, succ.connection_,
                                       succ.day_idx_, event_type::DEP);

            stops.emplace_back(
                static_cast<unsigned int>(++stop_index),
                get_node(current)->get_station()->id_,
                current.connection_->full_con_->a_track_,
                succ.connection_->full_con_->d_track_,
                current.connection_->event_time(event_type::ARR,
                                                current.day_idx_),
                succ.connection_->event_time(event_type::DEP, current.day_idx_),
                a_di.get_schedule_time(), d_di.get_schedule_time(),
                a_di.get_reason(), d_di.get_reason(), false, false);
          }
        }

        last_route_node = get_node(current);
        last_con = current.connection_;
        last_con_day_idx = current.day_idx_;
        break;
      }
    }

    ++it;
    if (it != end(labels)) {
      current = *it;
      auto next = it == end(labels) || std::next(it) == end(labels)
                      ? nullptr
                      : &(*std::next(it));
      current_state = next_state(current_state, &current, next);
    }
  }

  return ret;
}

}  // namespace output
}  // namespace routing
}  // namespace motis
