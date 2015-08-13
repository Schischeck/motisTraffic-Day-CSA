#include <iostream>
#include <sstream>
#include <algorithm>

#include "motis/realtime/waiting_edges.h"
#include "motis/realtime/realtime_schedule.h"
#include "motis/realtime/delay_info.h"
#include "motis/core/common/logging.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/station.h"

namespace motis {
namespace realtime {

using namespace motis::logging;

waiting_edges::~waiting_edges() {
  for (auto& e : _outgoing_edges) {
    for (auto& v : e.second) {
      delete v;
    }
    e.second.clear();
  }
  for (auto& e : _additional_outgoing_edges) {
    for (auto& v : e.second) {
      delete v;
    }
    e.second.clear();
  }
}

// for debugging
void waiting_edges::log_light_connection(const motis::light_connection* lc) {
  const motis::connection* con = lc->_full_con;
  const motis::connection_info* ci = con->con_info;
  LOG(debug) << "    | "
             << "d=" << lc->d_time << " (" << motis::format_time(lc->d_time)
             << ")"
             << " a=" << lc->a_time << " (" << motis::format_time(lc->a_time)
             << ")"
             << " family=" << ci->family << " ("
             << _rts._schedule.category_names[ci->family] << ")"
             << " train_nr=" << ci->train_nr
             << " wtr_category=" << _wtr.waiting_time_category(ci->family)
             << " service=" << ci->service;
}

void waiting_edges::create_waiting_edges() {
  // scoped_timer timer("create_waiting_edges");

  for (size_t station_index = 0;
       station_index < _rts._schedule.station_nodes.size(); station_index++) {
    const auto& station_node = _rts._schedule.station_nodes[station_index];
    const motis::station* station =
        _rts._schedule.stations[station_index].get();
    const int transfer_time = station->get_transfer_time();
    //    LOG(debug) << "station node:"
    //               << " index=" << station.index
    //               << " eva_nr=" << station.eva_nr
    //               << " name=" << station.name
    //               << " transfer_time=" << transfer_time;
    assert(station_index == station_node->_id);
    for (motis::node const* const route_node :
         station_node->get_route_nodes()) {
      //      LOG(debug) << "  route node:"
      //                 << " route=" << route_node->_route
      //                 << " id=" << route_node->_id
      //                 << " edges=" << route_node->_edges.size();

      // check that there is a foot edge to this station so that we can
      // leave the train

      bool found_exit_edge = false;
      for (const motis::edge& check_edge : route_node->_edges) {
        if (check_edge.type() == motis::edge::FOOT_EDGE &&
            check_edge._to.ptr() == station_node.get()) {
          found_exit_edge = true;
          break;
        }
      }
      if (!found_exit_edge) {
        continue;
      }

      // check arriving trains
      const motis::edge* previous_route_edge = _rts.get_prev_edge(route_node);
      if (previous_route_edge != nullptr && !previous_route_edge->empty()) {
        // check first lightconnection
        const motis::light_connection& first_feeder =
            previous_route_edge->_m._route_edge._conns[0];
        const int feeder_category = _wtr.waiting_time_category(
            first_feeder._full_con->con_info->family);

        // TODO: category does not have to be the same in all LCs of
        // a route edge!

        // skip further checks if no trains wait for this category
        if (!_wtr.other_trains_wait_for(feeder_category)) continue;

        // find departing trains that could wait
        for (const motis::edge& out_edge : station_node->_edges) {
          const motis::node* connector_route_node = out_edge.get_destination();
          if (out_edge.type() == motis::edge::FOOT_EDGE &&
              connector_route_node->is_route_node() &&
              connector_route_node != route_node &&
              route_node->_route != connector_route_node->_route) {

            for (const motis::edge& route_edge : connector_route_node->_edges) {
              if (route_edge.empty()) continue;

              create_waiting_edges(previous_route_edge, &route_edge,
                                   &first_feeder, feeder_category,
                                   transfer_time);
            }
          }
        }  // departing trains

      }  // arriving trains

    }  // route nodes
  }  // station nodes
}

void waiting_edges::create_waiting_edges(
    const motis::edge* feeder, const motis::edge* connector,
    const motis::light_connection* first_feeder_lc, int feeder_category,
    int station_transfer_time) {
  const int interchange_time = station_transfer_time;

  // TODO: category does not have to be the same in all LCs of
  // a route edge!

  const motis::light_connection* first_connector_lc =
      connector->get_connection(first_feeder_lc->a_time + interchange_time);

  if (first_connector_lc == nullptr) return;
  const int connector_category = _wtr.waiting_time_category(
      first_connector_lc->_full_con->con_info->family);

  const int waiting_time =
      _wtr.waiting_time(connector_category, feeder_category);

  if (waiting_time == 0) return;

  waiting_edge* we = nullptr;

  const motis::light_connection* feeder_lc = first_feeder_lc;
  while (feeder_lc != nullptr) {

    const motis::light_connection* connector_lc =
        connector->get_connection(feeder_lc->a_time + interchange_time);
    while (connector_lc != nullptr) {
      if (connector_lc->d_time - feeder_lc->a_time > 30) break;
      // ADD WAITING EDGE
      //      LOG(debug) << "    waiting edge between the following two LCs:"
      //                 << " (interchange time = " << interchange_time << ")";
      //      log_light_connection(feeder_lc);
      //      log_light_connection(connector_lc);

      if (we == nullptr) {
        we = new waiting_edge(feeder->_to->get_station()->_id,
                              feeder->_to->_route, connector->_to->_route,
                              waiting_time);
        store_waiting_edge(we);
      }

      we->_lc_pairs.emplace_back(feeder_lc->a_time, connector_lc->d_time,
                                 feeder_lc->_full_con->con_info->train_nr,
                                 connector_lc->_full_con->con_info->train_nr);

      connector_lc = connector->get_connection(connector_lc->d_time + 1);
    }

    // get next feeder
    feeder_lc = feeder->get_connection(feeder_lc->d_time + 1);
  }
}

void waiting_edges::store_waiting_edge(waiting_edge* we) {
  _outgoing_edges[std::make_pair(we->_station_index, we->_from_route_id)]
      .push_back(we);
  _incoming_edges[std::make_pair(we->_station_index, we->_to_route_id)]
      .push_back(we);
}

void waiting_edges::event_moved_to_new_route(const schedule_event& event,
                                             int32_t old_route_id,
                                             int32_t new_route_id) {
  const int station_index = event._station_index;

  if (_rts.is_debug_mode())
    LOG(debug) << "event_moved_to_new_route: " << event
               << ", old_route_id=" << old_route_id
               << ", new_route_id=" << new_route_id;

  // outgoing edges
  auto it = _outgoing_edges.find(std::make_pair(station_index, old_route_id));
  if (it != _outgoing_edges.end()) {
    for (waiting_edge* we : it->second) {
      std::vector<single_waiting_edge> events;
      we->add_connector_departure_events(event._schedule_time, events);
      assert(we->_from_route_id == old_route_id);
      for (const single_waiting_edge& swe : events) {
        move_to_new_route(swe, new_route_id, we->_to_route_id);
      }
    }
  }

  // incoming edges
  it = _incoming_edges.find(std::make_pair(station_index, old_route_id));
  if (it != _incoming_edges.end()) {
    for (waiting_edge* we : it->second) {
      std::vector<single_waiting_edge> events;
      we->add_feeder_arrival_events(event._schedule_time, events);
      assert(we->_to_route_id == old_route_id);
      for (const single_waiting_edge& swe : events) {
        move_to_new_route(swe, we->_from_route_id, new_route_id);
      }
    }
  }
}

void waiting_edges::add_additional_edge(
    const schedule_event& feeder_arrival,
    const schedule_event& connector_departure, int waiting_time) {
  std::vector<single_waiting_edge*>& ov =
      _additional_outgoing_edges[feeder_arrival];
  auto it = std::find_if(
      std::begin(ov), std::end(ov), [&](const single_waiting_edge* edge) {
        return edge->_connector_departure == connector_departure;
      });
  if (it == std::end(ov)) {
    single_waiting_edge* edge = new single_waiting_edge(
        feeder_arrival, connector_departure, waiting_time, nullptr);
    ov.push_back(edge);
    _additional_incoming_edges[connector_departure].push_back(edge);
  } else {
    (*it)->_waiting_time = waiting_time;
  }
}

void waiting_edges::remove_additional_edge(
    const schedule_event& feeder_arrival,
    const schedule_event& connector_departure) {
  auto out_it = _additional_outgoing_edges.find(feeder_arrival);
  if (out_it != std::end(_additional_outgoing_edges)) {
    auto& ov = out_it->second;
    auto edge_it = std::find_if(
        std::begin(ov), std::end(ov), [&](const single_waiting_edge* edge) {
          return edge->_connector_departure == connector_departure;
        });
    if (edge_it != std::end(ov)) {
      single_waiting_edge* swe = *edge_it;
      ov.erase(edge_it);
      if (ov.empty()) _additional_outgoing_edges.erase(out_it);

      auto in_it = _additional_incoming_edges.find(connector_departure);
      assert(in_it != std::end(_additional_incoming_edges));
      if (in_it != std::end(_additional_incoming_edges)) {
        auto& iv = in_it->second;
        iv.erase(std::remove(std::begin(iv), std::end(iv), swe), std::end(iv));
        if (iv.empty()) _additional_incoming_edges.erase(in_it);
      }

      delete swe;  // TODO
    }
  }
}

std::vector<uint32_t> waiting_edges::get_incoming_dependencies(
    uint32_t train_nr) const {
  std::vector<uint32_t> v;

  for (auto e : _incoming_edges) {
    std::vector<waiting_edge*>& edges = e.second;
    for (const waiting_edge* edge : edges) {
      for (const lc_pair& lcp : edge->_lc_pairs) {
        if (lcp._connector_train_nr == train_nr) {
          v.push_back(lcp._feeder_train_nr);
        }
      }
    }
  }

  return v;
}

void waiting_edges::write_dependency_graph(std::ostream& out) const {
  // TODO: duplicates
  out << "digraph waiting_edges {\n";
  for (auto e : _outgoing_edges) {
    std::vector<waiting_edge*>& edges = e.second;
    for (const waiting_edge* edge : edges) {
      for (const lc_pair& lcp : edge->_lc_pairs) {
        out << lcp._feeder_train_nr << " -> " << lcp._connector_train_nr
            << ";\n";
      }
    }
  }
  out << "}\n";
}

void waiting_edges::move_to_new_route(const single_waiting_edge& swe,
                                      int32_t new_feeder_route_id,
                                      int32_t new_connector_route_id) {
  if (_rts.is_debug_mode())
    LOG(debug) << "  move_to_new_route: SWE(arrival=" << swe._feeder_arrival
               << ", departure=" << swe._connector_departure
               << ", waiting_time=" << swe._waiting_time
               << ") - old WE route ids: "
               << swe._full_waiting_edge->_from_route_id << "->"
               << swe._full_waiting_edge->_to_route_id
               << " - new route ids: " << new_feeder_route_id << "->"
               << new_connector_route_id;

  auto& old_lc_pairs = swe._full_waiting_edge->_lc_pairs;
  old_lc_pairs.erase(
      std::remove(old_lc_pairs.begin(), old_lc_pairs.end(), lc_pair(swe)));

  waiting_edge* new_we =
      new waiting_edge(swe._feeder_arrival._station_index, new_feeder_route_id,
                       new_connector_route_id, swe._waiting_time);
  store_waiting_edge(new_we);

  new_we->_lc_pairs.emplace_back(swe._feeder_arrival._schedule_time,
                                 swe._connector_departure._schedule_time,
                                 swe._feeder_arrival._train_nr,
                                 swe._connector_departure._train_nr);
}

const std::vector<single_waiting_edge> waiting_edges::get_edges_from(
    const schedule_event& feeder_arrival, int32_t route_id) const {
  std::vector<single_waiting_edge> result;
  // from waiting time rules
  auto it = _outgoing_edges.find({feeder_arrival._station_index, route_id});
  if (it != _outgoing_edges.end()) {
    for (waiting_edge* we : it->second) {
      we->add_connector_departure_events(feeder_arrival._schedule_time, result);
    }
  }
  // additional waiting edges
  auto ait = _additional_outgoing_edges.find(feeder_arrival);
  if (ait != _additional_outgoing_edges.end()) {
    for (const single_waiting_edge* edge : ait->second) {
      result.push_back(*edge);
    }
  }
  return result;
}

const std::vector<single_waiting_edge> waiting_edges::get_edges_to(
    const schedule_event& connector_departure, int32_t route_id) const {
  std::vector<single_waiting_edge> result;
  // from waiting time rules
  auto it =
      _incoming_edges.find({connector_departure._station_index, route_id});
  if (it != _outgoing_edges.end()) {
    for (waiting_edge* we : it->second) {
      we->add_feeder_arrival_events(connector_departure._schedule_time, result);
    }
  }
  // additional waiting edges
  auto ait = _additional_incoming_edges.find(connector_departure);
  if (ait != _additional_incoming_edges.end()) {
    for (const single_waiting_edge* edge : ait->second) {
      result.push_back(*edge);
    }
  }
  return result;
}

void waiting_edge::add_feeder_arrival_events(
    motis::time connector_departure_time,
    std::vector<single_waiting_edge>& events) {
  auto it = std::lower_bound(std::begin(_lc_pairs), std::end(_lc_pairs),
                             connector_departure_time,
                             [](const lc_pair& lcp, const motis::time t) {
                               return lcp._connector_d_time < t;
                             });
  while (it != std::end(_lc_pairs) &&
         it->_connector_d_time == connector_departure_time) {
    events.emplace_back(schedule_event(_station_index, it->_feeder_train_nr,
                                       false, it->_feeder_a_time),
                        schedule_event(_station_index, it->_connector_train_nr,
                                       true, it->_connector_d_time),
                        _waiting_time, this);
    ++it;
  }
}

void waiting_edge::add_connector_departure_events(
    motis::time feeder_arrival_time, std::vector<single_waiting_edge>& events) {
  auto it = std::lower_bound(std::begin(_lc_pairs), std::end(_lc_pairs),
                             feeder_arrival_time,
                             [](const lc_pair& lcp, const motis::time t) {
                               return lcp._feeder_a_time < t;
                             });
  while (it != std::end(_lc_pairs) &&
         it->_feeder_a_time == feeder_arrival_time) {
    events.emplace_back(schedule_event(_station_index, it->_feeder_train_nr,
                                       false, it->_feeder_a_time),
                        schedule_event(_station_index, it->_connector_train_nr,
                                       true, it->_connector_d_time),
                        _waiting_time, this);
    ++it;
  }
}

}  // namespace realtime
}  // namespace motis
