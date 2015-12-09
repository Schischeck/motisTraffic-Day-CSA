#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/schedule.h"

namespace motis {
namespace reliability {

namespace graph_accessor {

inline edge const* get_departing_route_edge(node const& route_node) {
  for (auto& edge : route_node._edges) {
    if (!edge.empty()) {
      return &edge;
    }
  }
  return nullptr;
}

inline edge const* get_arriving_route_edge(node const& route_node) {
  for (auto& edge : route_node._incoming_edges) {
    if (!edge->empty()) {
      return edge;
    }
  }
  return nullptr;
}

/**
 * returns the latest light-connection that arrives not later
 * than 'departure_time'.
 * Note: this function assumes that departing_light_conn is not the
 * first departure of the route.
 * @param route_node is the tail node of departing_light_conn
 */
inline light_connection const& get_previous_light_connection(
    array<light_connection> const& arriving_light_connections,
    motis::time const departure_time) {
  if (arriving_light_connections.size() == 1) {
    return arriving_light_connections.front();
  }
  auto ub = std::upper_bound(
      arriving_light_connections.begin(), arriving_light_connections.end(),
      departure_time,
      [](unsigned int const dep_time, light_connection const& lc) {
        return dep_time < lc.a_time;
      });
  if (ub == arriving_light_connections.begin()) {
    return arriving_light_connections.front();
  } else if (ub == arriving_light_connections.end()) {
    return arriving_light_connections.back();
  }
  return *(--ub);
}

/* returns success, begin_time, end_time */
inline std::tuple<bool, time, time> get_feeder_time_interval(
    time const departure_time, duration change_time,
    duration const feeder_threshold) {
  change_time = std::max(change_time, (duration)1);
  time const begin_time =
      (departure_time <= feeder_threshold ? 0
                                          : departure_time - feeder_threshold);
  time const end_time = departure_time - change_time;
  bool const success =
      (begin_time <= end_time && departure_time >= change_time);

  return std::make_tuple(success, begin_time, end_time);
}

#define FEEDER_THRESHOLD 30 /* XXX */
struct feeder_info {
  node const* arrival_node_;
  light_connection const* light_conn_;
};
inline std::vector<feeder_info> get_all_potential_feeders(
    node const& route_node, motis::time const departure_time,
    duration const transfer_time) {
  std::vector<feeder_info> feeders;

  for (auto const in_edge : route_node._station_node->_incoming_edges) {
    // ignore transfer edge to route_node itself
    if (in_edge->_from->_id == route_node._id) {
      continue;
    }

    bool success;
    time time_begin, time_end;
    std::tie(success, time_begin, time_end) = get_feeder_time_interval(
        departure_time, transfer_time, FEEDER_THRESHOLD);
    if (success) {
      auto const& feeder_route_node = *in_edge->_from;
      auto const feeder_route_edge = get_arriving_route_edge(feeder_route_node);
      if (feeder_route_edge != nullptr) {
        auto& all_connections = feeder_route_edge->_m._route_edge._conns;
        for (unsigned int i = 0; i < all_connections.size(); i++) {
          if (all_connections[i].a_time >= time_begin &&
              all_connections[i].a_time <= time_end) {
            feeders.push_back({&feeder_route_node, &all_connections[i]});
          }
        }
      }
    }
  }  // end of for
  return feeders;
}

inline duration get_waiting_time(
    waiting_time_rules const& waiting_time_rules,
    light_connection const& feeder_light_conn,
    light_connection const& connecting_light_conn) {
  return (duration)waiting_time_rules.waiting_time(
      waiting_time_rules.waiting_time_category(
          connecting_light_conn._full_con->con_info->family),
      waiting_time_rules.waiting_time_category(
          feeder_light_conn._full_con->con_info->family));
}

inline std::string train_name(
    light_connection const& conn,
    std::vector<std::unique_ptr<category>> const& categories) {
  std::stringstream sst;
  sst << categories[conn._full_con->con_info->family]->name
      << conn._full_con->con_info->train_nr;
  return sst.str();
}

inline node const& get_first_route_node(node const& route_node) {
  node const* curr_node = &route_node;
  edge const* edge;
  while ((edge = get_arriving_route_edge(*curr_node)) != nullptr) {
    curr_node = edge->_from;
  }
  return *curr_node;
}

/* only for test-schedules: requires unique train numbers */
inline node const* get_first_route_node(schedule const& schedule,
                                        unsigned int const train_nr) {
  for (auto node : schedule.route_index_to_first_route_node) {
    assert(graph_accessor::get_departing_route_edge(*node));
    assert(graph_accessor::get_departing_route_edge(*node)
               ->_m._route_edge._conns.size() > 0);
    if (graph_accessor::get_departing_route_edge(*node)
            ->_m._route_edge._conns[0]
            ._full_con->con_info->train_nr == train_nr) {
      return node;
    }
  }
  return nullptr;
}

inline std::pair<light_connection const*, unsigned int> find_light_connection(
    edge const& route_edge, motis::time const departure_time,
    unsigned int const family, unsigned int const train_nr,
    std::string const& line_identifier) {
  if (route_edge.empty()) {
    return std::make_pair(nullptr, 0);
  }
  auto it = std::lower_bound(std::begin(route_edge._m._route_edge._conns),
                             std::end(route_edge._m._route_edge._conns),
                             light_connection(departure_time));
  while (it != std::end(route_edge._m._route_edge._conns) &&
         it->d_time == departure_time &&
         (it->_full_con->con_info->train_nr != train_nr ||
          it->_full_con->con_info->family != family ||
          it->_full_con->con_info->line_identifier != line_identifier)) {
    ++it;
  }
  return (it == std::end(route_edge._m._route_edge._conns) ||
          it->d_time != departure_time)
             ? std::make_pair(nullptr, 0)
             : std::make_pair(
                   it, it - std::begin(route_edge._m._route_edge._conns));
}

inline duration walking_duration(node const& tail_station,
                                 node const& head_station) {
  node const* foot_node = nullptr;
  for (auto e : tail_station._edges) {
    if (e._to->is_foot_node()) {
      foot_node = e._to;
      break;
    }
  }
  assert(foot_node);
  for (auto e : foot_node->_edges) {
    if (e._to->_id == head_station._id) {
      return e._m._foot_edge._time_cost;
    }
  }
  assert(false);
  return 0;
}

inline duration get_interchange_time(node const& arrival_node_feeder,
                                     node const& departure_node_departing_train,
                                     schedule const& schedule) {
  if (arrival_node_feeder._station_node->_id !=
      departure_node_departing_train._station_node->_id) {
    return walking_duration(*arrival_node_feeder._station_node,
                            *departure_node_departing_train._station_node);
  }
  return schedule.stations[departure_node_departing_train._station_node->_id]
      ->transfer_time;
}

/* note: category-names are not unique */
inline std::pair<bool, int> find_family(
    std::vector<std::unique_ptr<category>> const& categories,
    std::string const& category_name) {
  auto const cat_it =
      std::find_if(categories.begin(), categories.end(),
                   [category_name](std::unique_ptr<category> const& cat) {
                     return cat->name == category_name;
                   });

  if (cat_it == categories.end()) {
    return std::make_pair(false, 0);
  }
  return std::make_pair(true, cat_it - categories.begin());
}

inline void print_route_edge(edge const& route_edge, schedule const& schedule,
                             std::ostream& os) {
  os << "route-edge from "
     << schedule.stations[route_edge._from->get_station()->_id]->name
     << "(route-node-id: " << route_edge._from->_id << ") to "
     << schedule.stations[route_edge._to->get_station()->_id]->name
     << "(route-node-id: " << route_edge._to->_id << "):\n";
  unsigned int light_connection_idx = 0;
  for (auto const& light_connection : route_edge._m._route_edge._conns) {
    os << schedule.stations[route_edge._from->_station_node->_id]->name << "/"
       << schedule.stations[route_edge._from->_station_node->_id]->eva_nr << " "
       << format_time(light_connection.d_time) << "--"
       << schedule.categories[light_connection._full_con->con_info->family]
              ->name
       << light_connection._full_con->con_info->train_nr << ","
       << light_connection._full_con->con_info->line_identifier << "("
       << light_connection_idx++ << ")->"
       << format_time(light_connection.a_time) << " "
       << schedule.stations[route_edge._to->_station_node->_id]->name << "/"
       << schedule.stations[route_edge._to->_station_node->_id]->eva_nr
       << std::endl;
  }
}

inline void print_route(node const* const first_route_node,
                        schedule const& schedule, std::ostream& os) {
  node const* node = first_route_node;
  edge const* edge = nullptr;
  while ((edge = get_departing_route_edge(*node)) != nullptr) {
    print_route_edge(*edge, schedule, os);
    node = edge->_to;
  }
  os << std::endl;
}

}  // namespace graph_accessor
}  // namespace reliability
}  // namespace motis
