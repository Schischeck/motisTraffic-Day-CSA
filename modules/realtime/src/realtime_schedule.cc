#include <iostream>
#include <tuple>
#include <algorithm>

#include "motis/realtime/realtime_schedule.h"
#include "motis/core/common/logging.h"

namespace motis {
namespace realtime {

using namespace motis::logging;

realtime_schedule::realtime_schedule(motis::schedule& schedule)
    : _schedule(schedule),
      _waiting_edges(*this, schedule.waiting_time_rules_),
      _delay_info_manager(*this),
      _delay_propagator(*this),
      _graph_updater(*this),
      _message_handler(*this),
      _message_output(*this),
      _tracking(*this),
      _stats(),
      _debug_mode(false) {
  _waiting_edges.create_waiting_edges();

  // init max route id
  _max_route_id = 0;
  for (const auto& station : _schedule.station_nodes) {
    for (const auto& edge : station->_edges) {
      if (edge._to->is_route_node()) {
        auto route_id = edge._to->_route;
        if (route_id > _max_route_id) {
          _max_route_id = route_id;
        }
      }
    }
  }
}

std::pair<motis::node*, motis::light_connection*>
realtime_schedule::locate_event(const graph_event& event_id) const {
  const int32_t route_id = event_id._route_id;

  for (node* route_node :
       _schedule.station_nodes[event_id._station_index]->get_route_nodes()) {
    if (route_id != -1 && route_node->_route != route_id) continue;

    motis::edge* route_edge = event_id.departure() ? get_next_edge(route_node)
                                                   : get_prev_edge(route_node);

    if (route_edge == nullptr) continue;

    motis::light_connection* lc =
        event_id.departure()
            ? get_connection_with_departure_time(route_edge, event_id.time(),
                                                 event_id._train_nr)
            : get_connection_with_arrival_time(route_edge, event_id.time(),
                                               event_id._train_nr);

    if (lc != nullptr &&
        lc->_full_con->con_info->train_nr == event_id._train_nr) {
      return std::make_pair(route_node, lc);
    }
  }

  return std::make_pair(nullptr, nullptr);
}

motis::node* realtime_schedule::locate_route_node(unsigned station_index,
                                                  int32_t route_id) const {
  for (motis::node* route_node :
       _schedule.station_nodes[station_index]->get_route_nodes()) {
    if (route_node->_route == route_id) return route_node;
  }
  return nullptr;
}

graph_event realtime_schedule::get_previous_graph_event(
    const graph_event& ref_graph_event) const {
  motis::node* ref_node;
  motis::light_connection* ref_lc;
  std::tie(ref_node, ref_lc) = locate_event(ref_graph_event);

  if (ref_node != nullptr && ref_lc != nullptr) {
    if (ref_graph_event.departure()) {
      // find previous arrival (may exist)
      motis::edge* prev_edge = get_prev_edge(ref_node);
      if (prev_edge != nullptr) {
        const motis::light_connection* prev_lc =
            get_last_connection_with_arrival_before(prev_edge,
                                                    ref_graph_event.time());
        if (prev_lc != nullptr) {
          return graph_event(ref_graph_event._station_index,
                             prev_lc->_full_con->con_info->train_nr, false,
                             prev_lc->a_time, ref_node->_route);
        }
      }
    } else {
      // get previous departure (must exist)
      const motis::node* prev_node = get_prev_node(ref_node);
      assert(prev_node != nullptr);
      uint32_t prev_station_id = prev_node->get_station()->_id;
      return graph_event(prev_station_id, ref_lc->_full_con->con_info->train_nr,
                         true, ref_lc->d_time, ref_node->_route);
    }
  }

  return graph_event();
}

schedule_event realtime_schedule::get_previous_schedule_event(
    const graph_event& ref_graph_event) const {
  return get_schedule_event(get_previous_graph_event(ref_graph_event));
}

graph_event realtime_schedule::get_next_graph_event(
    const graph_event& ref_graph_event) const {
  motis::node* ref_node;
  motis::light_connection* ref_lc;
  std::tie(ref_node, ref_lc) = locate_event(ref_graph_event);
  if (ref_node != nullptr && ref_lc != nullptr) {
    if (ref_graph_event.departure()) {
      // get next arrival (must exist)
      uint32_t next_station_id =
          get_next_edge(ref_node)->_to->get_station()->_id;
      return graph_event(next_station_id, ref_lc->_full_con->con_info->train_nr,
                         false, ref_lc->a_time, ref_node->_route);
    } else {
      // get next departure (may exist)
      motis::edge* next_edge = get_next_edge(ref_node);
      if (next_edge != nullptr) {
        const motis::light_connection* next_lc =
            next_edge->get_connection(ref_lc->a_time);
        if (next_lc != nullptr) {
          return graph_event(ref_node->get_station()->_id,
                             next_lc->_full_con->con_info->train_nr, true,
                             next_lc->d_time, ref_node->_route);
        }
      }
    }
  }

  return graph_event();
}

schedule_event realtime_schedule::get_next_schedule_event(
    const graph_event& ref_graph_event) const {
  return get_schedule_event(get_next_graph_event(ref_graph_event));
}

schedule_event realtime_schedule::get_schedule_event(
    const graph_event& graph_event) const {
  if (!graph_event.found()) {
    return schedule_event(graph_event._station_index, graph_event._train_nr,
                          graph_event._departure, graph_event._current_time);
  }

  delay_info* delay_info = _delay_info_manager.get_delay_info(graph_event);
  if (delay_info != nullptr) {
    return delay_info->sched_ev();
  } else {
    return schedule_event(graph_event._station_index, graph_event._train_nr,
                          graph_event._departure, graph_event._current_time);
  }
}

graph_event realtime_schedule::get_graph_event(
    const schedule_event& schedule_event) const {
  delay_info* di = _delay_info_manager.get_delay_info(schedule_event);
  if (di == nullptr) {
    return graph_event(schedule_event);
  } else {
    return di->graph_ev();
  }
}

std::pair<motis::node*, motis::light_connection*>
realtime_schedule::locate_start_of_train(
    motis::node* ref_node, motis::light_connection* ref_lc) const {

  motis::node* route_node = ref_node;
  motis::light_connection* lc = ref_lc;

  while (true) {
    motis::edge* prev_edge = get_prev_edge(route_node);
    if (prev_edge == nullptr) break;
    motis::node* prev_node = get_prev_node(route_node);
    motis::light_connection* prev_lc =
        get_last_connection_with_arrival_before(prev_edge, lc->d_time);

    if (prev_lc != nullptr) {
      route_node = prev_node;
      lc = prev_lc;
    } else {
      // partial train (schedule bounds)
      break;
    }
  }

  return std::make_pair(route_node, lc);
}

std::tuple<schedule_event, graph_event, motis::node*, motis::light_connection*>
realtime_schedule::locate_start_of_train(
    const schedule_event& ref_event) const {
  motis::node* route_node;
  motis::light_connection* lc;
  std::tie(route_node, lc) = locate_event(get_graph_event(ref_event));
  if (route_node != nullptr && lc != nullptr) {
    if (ref_event.arrival()) {
      route_node = get_prev_node(route_node);
      assert(route_node != nullptr);
    }
    std::tie(route_node, lc) = locate_start_of_train(route_node, lc);
    assert(route_node != nullptr);
    assert(lc != nullptr);
    graph_event graph_start_event(route_node->get_station()->_id,
                                  lc->_full_con->con_info->train_nr, true,
                                  lc->d_time, route_node->_route);
    schedule_event schedule_start_event = get_schedule_event(graph_start_event);
    return std::make_tuple(schedule_start_event, graph_start_event, route_node,
                           lc);
  } else {
    return std::make_tuple(schedule_event(), graph_event(), nullptr, nullptr);
  }
}

std::vector<std::tuple<node*, schedule_event, schedule_event>>
realtime_schedule::get_train_events(const schedule_event& start_event) const {
  std::vector<std::tuple<node*, schedule_event, schedule_event>> v;

  if (start_event.found()) {
    // first departure
    node* route_node;
    edge* route_edge;
    light_connection* lc;
    std::tie(route_node, lc) = locate_event(get_graph_event(start_event));
    if (route_node == nullptr) {
      LOG(error) << "event not found: " << start_event << " -> "
                 << get_graph_event(start_event);
    }
    assert(route_node != nullptr);
    route_edge = get_next_edge(route_node);

    v.emplace_back(route_node, schedule_event(), start_event);

    bool single_train_route = is_single_train_route(route_node);

    motis::light_connection* last_lc = lc;
    while (route_edge != nullptr) {
      // arrival
      route_node = route_edge->_to;
      graph_event graph_event(route_node->get_station()->_id,
                              lc->_full_con->con_info->train_nr, false,
                              lc->a_time, route_node->_route);
      schedule_event arrival_event = get_schedule_event(graph_event);

      // departure
      schedule_event departure_event;
      route_edge = get_next_edge(route_node);
      if (route_edge != nullptr && !route_edge->empty()) {
        if (single_train_route) {
          lc = &route_edge->_m._route_edge._conns[0];
        } else {
          lc = route_edge->get_connection(last_lc->a_time);
        }
        graph_event._departure = true;
        graph_event._current_time = lc->d_time;
        graph_event._train_nr = lc->_full_con->con_info->train_nr;
        departure_event = get_schedule_event(graph_event);
        last_lc = lc;
      } else {
        departure_event._departure = true;
      }
      v.emplace_back(route_node, arrival_event, departure_event);
    }
  }

  return v;
}

schedule_event realtime_schedule::find_departure_event(uint32_t train_nr,
                                                       int day_index) const {
  if (is_debug_mode())
    LOG(info) << "searching departure event for train " << train_nr
              << " on day " << day_index << " ("
              << _schedule.station_nodes.size() << " stations)";
  for (const auto& station : _schedule.station_nodes) {
    for (const motis::node* route_node : station->get_route_nodes()) {
      const motis::edge* edge = get_next_edge(route_node);
      if (edge == nullptr) continue;
      for (const motis::light_connection& lc : edge->_m._route_edge._conns) {
        if (lc._full_con->con_info->train_nr == train_nr &&
            lc.d_time / MINUTES_A_DAY == day_index) {
          return get_schedule_event(graph_event(station->_id, train_nr, true,
                                                lc.d_time, route_node->_route));
        }
      }
    }
  }
  return schedule_event();
}

bool realtime_schedule::event_exists(const schedule_event& sched_event,
                                     graph_event* ge_out) const {
  delay_info* di = _delay_info_manager.get_delay_info(sched_event);
  if (di != nullptr /*&& !di->_canceled*/) {
    if (ge_out != nullptr) {
      *ge_out = di->graph_ev();
    }
    return true;
  }
  motis::node* route_node;
  motis::light_connection* lc;
  std::tie(route_node, lc) = locate_event(graph_event(sched_event));
  if (route_node != nullptr && lc != nullptr) {
    graph_event ge(route_node->get_station()->_id,
                   lc->_full_con->con_info->train_nr, sched_event.departure(),
                   sched_event.departure() ? lc->d_time : lc->a_time,
                   route_node->_route);
    if (ge_out != nullptr) {
      *ge_out = ge;
    }
    di = _delay_info_manager.get_delay_info(ge);
    if (di != nullptr) {
      return di->sched_ev() == sched_event;
    } else {
      return true;
    }
  }
  return false;
}

void realtime_schedule::track_train(uint32_t train_nr) {
  if (is_tracked(train_nr)) return;
  LOG(debug) << "tracking train " << train_nr;
  _tracked_trains.push_back(train_nr);
  // std::vector<uint32_t> dependencies =
  //     _waiting_edges.get_incoming_dependencies(train_nr);
  // if (!dependencies.empty()) {
  //   auto& out = LOG(debug) << "also tracking trains because of waiting edges:
  //   ";
  //   std::copy(dependencies.begin(), dependencies.end(),
  //             std::ostream_iterator<uint32_t>(out, " "));
  //   for (auto t : dependencies) track_train(t);
  // }
}

bool realtime_schedule::is_tracked(uint32_t train_nr) const {
  return std::find(_tracked_trains.begin(), _tracked_trains.end(), train_nr) !=
         _tracked_trains.end();
}

bool realtime_schedule::is_single_train_route(
    const motis::node* start_node) const {
  assert(start_node != nullptr);
  if (get_prev_node(start_node) != nullptr) {
    return false;
  }

  return get_next_edge(start_node)->_m._route_edge._conns.size() == 1;
}

//////////////////////////////////////////////////////////

motis::node* realtime_schedule::get_prev_node(motis::node* route_node) const {
  motis::edge* edge = get_prev_edge(route_node);
  assert(edge == nullptr || edge->_from != route_node);
  assert(edge == nullptr || edge->_from->_route == route_node->_route);
  return edge != nullptr ? edge->_from : nullptr;
}

const motis::node* realtime_schedule::get_prev_node(
    const motis::node* route_node) const {
  const motis::edge* edge = get_prev_edge(route_node);
  assert(edge == nullptr || edge->_from != route_node);
  assert(edge == nullptr || edge->_from->_route == route_node->_route);
  return edge != nullptr ? edge->_from : nullptr;
}

motis::edge* realtime_schedule::get_prev_edge(motis::node* route_node) const {
  for (auto& e : route_node->_incoming_edges) {
    if (e != nullptr && e->type() == motis::edge::ROUTE_EDGE) {
      assert(e->_from != nullptr);
      if (e->_from->_route != route_node->_route) {
        LOG(error) << "broken edge?? route_node=" << route_node
                   << ", edge=" << e << ", from=" << e->_from;
        LOG(error) << "expected route=" << route_node->_route
                   << ", incoming edge from route=" << e->_from->_route
                   << " with type=" << e->type_str() << " "
                   << e->_m._route_edge._conns.size() << " connections";
        for (auto& lc : e->_m._route_edge._conns) {
          LOG(error) << "LC: train_nr=" << lc._full_con->con_info->train_nr
                     << ", d_time=" << motis::format_time(lc.d_time)
                     << ", a_time=" << motis::format_time(lc.a_time);
        }
      }
      assert(e->_from->_route == route_node->_route);
      return e;
    }
  }
  return nullptr;
}

const motis::edge* realtime_schedule::get_prev_edge(
    const motis::node* route_node) const {
  for (auto& e : route_node->_incoming_edges) {
    if (e != nullptr && e->type() == motis::edge::ROUTE_EDGE) {
      assert(e->_from != nullptr);
      assert(e->_from->_route == route_node->_route);
      return e;
    }
  }
  return nullptr;
}

motis::edge* realtime_schedule::get_next_edge(motis::node* route_node) const {
  for (auto& edge : route_node->_edges) {
    if (edge.type() == motis::edge::ROUTE_EDGE) {
      assert(edge._to->_route == route_node->_route);
      return &edge;
    }
  }
  return nullptr;
}

const motis::edge* realtime_schedule::get_next_edge(
    const motis::node* route_node) const {
  for (auto& edge : route_node->_edges) {
    if (edge.type() == motis::edge::ROUTE_EDGE) {
      assert(edge._to->_route == route_node->_route);
      return &edge;
    }
  }
  return nullptr;
}

motis::node* realtime_schedule::get_next_node(motis::node* route_node) {
  motis::edge* edge = get_next_edge(route_node);
  if (edge != nullptr) {
    assert(edge->_to->_route == route_node->_route);
    return edge->_to;
  } else {
    return nullptr;
  }
}

motis::node* realtime_schedule::get_start_node(motis::node* route_node) {
  assert(route_node->is_route_node());
  motis::node* prev = get_prev_node(route_node);
  while (prev != nullptr) {
    route_node = prev;
    prev = get_prev_node(route_node);
  }
  return route_node;
}

//////////////////////////////////////////////////////////

motis::light_connection* realtime_schedule::get_connection_with_departure_time(
    motis::edge* route_edge, motis::time departure_time,
    uint32_t train_nr) const {
  if (route_edge->empty()) return nullptr;

  auto it = std::lower_bound(std::begin(route_edge->_m._route_edge._conns),
                             std::end(route_edge->_m._route_edge._conns),
                             light_connection(departure_time));

  while (it != std::end(route_edge->_m._route_edge._conns) &&
         it->d_time == departure_time &&
         it->_full_con->con_info->train_nr != train_nr) {
    it++;
  }

  return (it == std::end(route_edge->_m._route_edge._conns) ||
          it->d_time != departure_time)
             ? nullptr
             : it;
}

motis::light_connection* realtime_schedule::get_connection_with_arrival_time(
    motis::edge* route_edge, motis::time arrival_time,
    uint32_t train_nr) const {
  if (route_edge->empty()) return nullptr;

  auto it = std::lower_bound(
      std::begin(route_edge->_m._route_edge._conns),
      std::end(route_edge->_m._route_edge._conns), arrival_time,
      [](const motis::light_connection& elm, const motis::time t) {
        return elm.a_time < t;
      });

  while (it != std::end(route_edge->_m._route_edge._conns) &&
         it->a_time == arrival_time &&
         it->_full_con->con_info->train_nr != train_nr) {
    it++;
  }

  return (it == std::end(route_edge->_m._route_edge._conns) ||
          it->a_time != arrival_time)
             ? nullptr
             : it;
}

motis::light_connection*
realtime_schedule::get_last_connection_with_arrival_before(
    motis::edge* route_edge, motis::time max_time) const {
  if (route_edge->empty())
    return nullptr;
  else if (route_edge->_m._route_edge._conns.size() == 1)
    return &route_edge->_m._route_edge._conns[0];

  auto it =
      std::lower_bound(std::begin(route_edge->_m._route_edge._conns),
                       std::end(route_edge->_m._route_edge._conns), max_time,
                       [](const motis::light_connection& elm,
                          const motis::time t) { return elm.a_time < t; });

  motis::light_connection* lc = nullptr;
  if (it == std::end(route_edge->_m._route_edge._conns)) {
    lc = &route_edge->_m._route_edge._conns.back();
  } else {
    if (it->a_time == max_time) {
      lc = it;
    } else if (it == std::begin(route_edge->_m._route_edge._conns)) {
      lc = nullptr;
    } else {
      lc = it - 1;
    }
  }

  return lc;
}

}  // namespace realtime
}  // namespace td
