#include <cassert>
#include <utility>
#include <vector>
#include <tuple>
#include <unordered_set>
#include <algorithm>
#include <iostream>

#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/price.h"
#include "motis/core/common/logging.h"

#include "motis/realtime/graph_updater.h"
#include "motis/realtime/realtime_schedule.h"

namespace motis {
namespace realtime {

using namespace motis::logging;

void graph_updater::perform_updates(std::vector<delay_info_update>& updates) {
  operation_timer timer(_rts._stats._graph_updater);
  std::sort(updates.begin(), updates.end());

  bool old_debug = _rts._debug_mode;
  if (!updates.empty() &&
      _rts.is_tracked(updates[0]._delay_info->sched_ev()._train_nr)) {
    _rts._debug_mode = true;
  }

  if (_rts.is_debug_mode()) {
    LOG(debug) << "performing graph updates...";
    for (const auto& u : updates) LOG(debug) << "-- " << u;
    if (!check_updates(updates)) {
      LOG(warn) << "invalid updates! skipping...";
      return;
    }
    LOG(debug) << "processing updates...";
  }

  while (!updates.empty()) {
    const delay_info_update& update = updates[0];
    delay_info* di = update._delay_info;
    graph_event ge = di->graph_ev();
    motis::node* route_node;
    motis::light_connection* lc;
    std::tie(route_node, lc) = _rts.locate_event(ge);

    if (route_node == nullptr && di->canceled()) {
      updates.erase(updates.begin());
      continue;
    }

    if (route_node == nullptr || lc == nullptr) {
      LOG(warn) << "did not find event: " << ge << ", update: " << update;

      schedule_event orig_event = di->sched_ev();
      uint32_t train_nr = orig_event._train_nr;
      schedule_event ref_event = _rts.find_departure_event(
          orig_event._train_nr, orig_event._schedule_time / MINUTES_A_DAY);
      if (ref_event.found()) {
        LOG(warn) << "found a departure event for train "
                  << orig_event._train_nr << " on day "
                  << orig_event._schedule_time / MINUTES_A_DAY << ": "
                  << ref_event;
        schedule_event start_event;
        motis::node* start_node;
        std::tie(start_event, std::ignore, start_node, std::ignore) =
            _rts.locate_start_of_train(ref_event);
        LOG(warn) << "start event of the train: " << start_event;
        auto events = _rts.get_train_events(start_event);
        for (auto& e : events) {
          motis::node* route_node;
          schedule_event sarr, sdep;
          std::tie(route_node, sarr, sdep) = e;
          graph_event garr = _rts.get_graph_event(sarr);
          graph_event gdep = _rts.get_graph_event(sdep);
          if (sarr.found()) LOG(info) << " ** a=" << sarr << " / " << garr;
          if (sdep.found()) LOG(info) << " ** d=" << sdep << " / " << gdep;
        }
        dump_route(
            start_node,
            "route containing the train" /*, start_event._schedule_time / MINUTES_A_DAY*/);
      } else {
        LOG(warn) << "did not find a departure event for train "
                  << orig_event._train_nr << " on day "
                  << orig_event._schedule_time / MINUTES_A_DAY;
      }
      LOG(info) << "searching all delay_infos for the train...";
      std::vector<delay_info*> dis;
      std::copy_if(_rts._delay_info_manager._delay_infos.begin(),
                   _rts._delay_info_manager._delay_infos.end(),
                   std::back_inserter(dis), [=](const delay_info* di) {
                     return di->_schedule_event._train_nr == train_nr;
                   });
      std::sort(dis.begin(), dis.end(),
                [](const delay_info* a, const delay_info* b) {
                  return a->_schedule_event._schedule_time <
                         b->_schedule_event._schedule_time;
                });
      for (const delay_info* di : dis) {
        LOG(info) << " ** " << *di;
      }
    }
    assert(route_node != nullptr && lc != nullptr);
    if (ge.arrival()) {
      route_node = _rts.get_prev_node(route_node);
      assert(route_node != nullptr);
    }

    if (_rts.is_debug_mode()) {
      dump_route(route_node, "route before update", lc->d_time / MINUTES_A_DAY);
    }

    auto n_before = updates.size();
    update_train_times(updates, route_node, lc);
    auto n_after = updates.size();

    if (_rts.is_debug_mode()) {
      dump_route(route_node, "route after update", lc->d_time / MINUTES_A_DAY);
    }

    if (n_after == n_before) {
      LOG(error) << "update_train_times did not process " << updates[0];
      if (!_rts.is_debug_mode()) {
        // retry with debug output
        _rts._debug_mode = true;
        continue;
      }
    }
    assert(n_after < n_before);
  }

  _rts._debug_mode = old_debug;
}

void graph_updater::update_train_times(std::vector<delay_info_update>& updates,
                                       motis::node* route_node,
                                       motis::light_connection* lc) {
  motis::node* new_route = nullptr;

  _rts._stats._ops.updater.time_updates++;

  if (_rts.is_debug_mode()) {
    LOG(debug) << "update_train_times: " << updates[0] << ", " << updates.size()
               << " updates";
    for (auto& u : updates) LOG(debug) << "-- " << u;
  }

  assert(check_route(route_node, false));

  motis::node* start_node;
  motis::light_connection* start_lc;
  std::tie(start_node, start_lc) = _rts.locate_start_of_train(route_node, lc);

  graph_train_info gti = get_graph_train_info(start_node, start_lc, updates);

  if (!gti._times_valid) {
    LOG(warn) << "ignoring updates for this train because of invalid times:";
    LOG(warn) << gti;
    return;
  } else if (_rts.is_debug_mode()) {
    LOG(debug) << "update_train_times:";
    LOG(debug) << gti;
  }

  if (gti._extract_required) {
    if (_rts.is_debug_mode()) LOG(debug) << "extract route required";
    gti = extract_route(gti);
    new_route = gti._edges[0]._from_route_node;
  }

  for (const graph_train_edge& e : gti._edges) {
    if (e._dep_update.valid()) {
      graph_event gdep(e._from_route_node->get_station()->_id,
                       e._lc->_full_con->con_info->train_nr, true,
                       e._lc->d_time, e._from_route_node->_route);
      delay_info* di = _rts._delay_info_manager.get_delay_info(gdep);
      if (di == nullptr) {
        LOG(error) << "delay info not found for " << gdep
                   << ", update: " << e._dep_update;
        LOG(error) << gti;
      }
      e._lc->d_time = e._dep_update._new_time;
      _rts._delay_info_manager.update_delay_info(&e._dep_update);
    }
    if (e._arr_update.valid()) {
      graph_event garr(
          _rts.get_next_edge(e._from_route_node)->_to->get_station()->_id,
          e._lc->_full_con->con_info->train_nr, false, e._lc->a_time,
          e._from_route_node->_route);
      assert(_rts._delay_info_manager.get_delay_info(garr) != nullptr);

      e._lc->a_time = e._arr_update._new_time;
      _rts._delay_info_manager.update_delay_info(&e._arr_update);
    }
  }

  if (_rts.is_debug_mode()) {
    LOG(debug) << "train times update, " << updates.size()
               << " updates remaining";
    if (new_route != nullptr)
      dump_route(new_route, "extracted and updated route");
  }

  assert(check_route(route_node, true));
  if (new_route != nullptr) assert(check_route(new_route, true));
}

std::pair<std::vector<std::tuple<motis::node*, schedule_event, schedule_event>>,
          modified_train*>
graph_updater::get_current_events_and_make_modified_train(
    schedule_event ref_event, bool get_current_events) {
  operation_timer timer(_rts._stats._graph_updater);
  modified_train* mt = _rts._modified_train_manager.train_with_event(ref_event);
  std::vector<std::tuple<motis::node*, schedule_event, schedule_event>>
      current_events;

  if (_rts.is_debug_mode())
    LOG(debug) << "get_current_events_and_make_modified_train(" << ref_event
               << ", " << get_current_events << ")";

  if (mt == nullptr) {
    motis::node* route_node;
    motis::light_connection* lc;
    schedule_event start_event;
    std::tie(start_event, std::ignore, route_node, lc) =
        _rts.locate_start_of_train(ref_event);
    if (_rts.is_debug_mode())
      LOG(debug) << "-- mt not found, start of train: start_event="
                 << start_event << ", route_node=" << route_node;
    if (route_node == nullptr || lc == nullptr)
      return {current_events, nullptr};
    if (get_current_events) current_events = _rts.get_train_events(start_event);
    mt = _rts._modified_train_manager.train_with_route_id(route_node->_route);
    if (mt == nullptr) {
      if (_rts.is_debug_mode()) LOG(debug) << "-- creating new mt";
      mt = make_modified_train(route_node, lc);
    } else {
      if (_rts.is_debug_mode())
        LOG(debug) << "-- mt for start event found: "
                   << mt->_current_start_event << " (route "
                   << mt->_new_route_id << ")";
    }
  } else if (get_current_events) {
    if (_rts.is_debug_mode())
      LOG(debug) << "-- mt already exists, start event: "
                 << mt->_current_start_event;
    current_events = _rts.get_train_events(mt->_current_start_event);
  }

  return {current_events, mt};
}

graph_train_info graph_updater::get_graph_train_info(
    motis::node* start_node, motis::light_connection* start_lc,
    std::vector<delay_info_update>& updates) {
  assert(start_node != nullptr && start_lc != nullptr);
  graph_train_info r;

  bool single_train_route = _rts.is_single_train_route(start_node);

  motis::node* route_node = start_node;
  motis::light_connection* lc = start_lc;
  while (lc != nullptr) {
    motis::edge* current_edge = _rts.get_next_edge(route_node);
    assert(current_edge != nullptr);
    motis::node* next_route_node = current_edge->_to;
    assert(next_route_node != nullptr);

    graph_event gdep(route_node->get_station()->_id,
                     lc->_full_con->con_info->train_nr, true, lc->d_time,
                     route_node->_route);
    graph_event garr(next_route_node->get_station()->_id, gdep._train_nr, false,
                     lc->a_time, gdep._route_id);

    motis::time new_dep_time = gdep._current_time;
    motis::time new_arr_time = garr._current_time;

    auto dit = std::find_if(updates.begin(), updates.end(),
                            [&](const delay_info_update& u) {
                              return u._delay_info->graph_ev() == gdep;
                            });
    delay_info_update diu_dep;
    if (dit != updates.end()) {
      diu_dep = *dit;
      new_dep_time = diu_dep._new_time;
      updates.erase(dit);
      if (_rts.is_debug_mode())
        LOG(debug) << "found update for departure event: " << gdep << ": "
                   << diu_dep;
      assert(diu_dep.valid());
    } else if (_rts.is_debug_mode()) {
      LOG(debug) << "found no update for departure event: " << gdep;
    }

    auto ait = std::find_if(updates.begin(), updates.end(),
                            [&](const delay_info_update& u) {
                              return u._delay_info->graph_ev() == garr;
                            });
    delay_info_update diu_arr;
    if (ait != updates.end()) {
      diu_arr = *ait;
      new_arr_time = diu_arr._new_time;
      updates.erase(ait);
      if (_rts.is_debug_mode())
        LOG(debug) << "found update for arrival event: " << garr << ": "
                   << diu_arr;
      assert(diu_arr.valid());
    } else if (_rts.is_debug_mode()) {
      LOG(debug) << "found no update for arrival event: " << garr;
    }

    r._edges.emplace_back(route_node, lc, diu_dep, diu_arr);

    // check if train must be extracted from the route
    if (!r._extract_required && !single_train_route) {

      // get the previous and next connection of this route edge
      //      const light_connection* prev_lc =
      //          current_edge->get_connection(lc->d_time - 1, nullptr);
      //      const light_connection* next_lc =
      //          current_edge->get_connection(lc->d_time + 1, nullptr);
      //      if (prev_lc == lc) prev_lc = nullptr;

      const motis::light_connection* prev_lc = lc - 1;
      if (prev_lc < current_edge->_m._route_edge._conns.begin())
        prev_lc = nullptr;
      const light_connection* next_lc = lc + 1;
      if (next_lc >= current_edge->_m._route_edge._conns.end())
        next_lc = nullptr;

      // check if the LC timestamps are valid for this route or if it needs
      // to be moved into a new route
      bool extract = false;

      // check against previous and next LC in this route edge
      extract |= (prev_lc != nullptr) && (new_dep_time <= prev_lc->d_time);
      extract |= (next_lc != nullptr) && (new_dep_time >= next_lc->d_time);
      extract |= (prev_lc != nullptr) && (new_arr_time <= prev_lc->a_time);
      extract |= (next_lc != nullptr) && (new_arr_time >= next_lc->a_time);

      motis::edge* prev_edge = _rts.get_prev_edge(route_node);
      if (prev_edge != nullptr) {
        prev_lc = _rts.get_last_connection_with_arrival_before(prev_edge,
                                                               new_dep_time);
        if (prev_lc != nullptr) extract |= !is_same_train(lc, prev_lc);
      }

      r._extract_required = extract;
    }

    route_node = next_route_node;
    motis::edge* next_edge = _rts.get_next_edge(next_route_node);
    if (next_edge != nullptr) {
      lc = next_edge->get_connection(lc->a_time);
    } else {
      lc = nullptr;
    }
  }

  motis::time last_time = 0;
  for (const auto& e : r._edges) {
    motis::time dep = e.new_departure_time();
    motis::time arr = e.new_arrival_time();
    if (dep < last_time || arr < dep) {
      LOG(warn) << "gti: invalid time in " << e << ": "
                << "last_time=" << motis::format_time(last_time)
                << ", dep=" << motis::format_time(dep)
                << ", arr=" << motis::format_time(arr);
      r._times_valid = false;
      break;
    }
    last_time = arr;
  }

  return r;
}

graph_train_info graph_updater::extract_route(const graph_train_info& gti) {
  if (gti._edges.empty()) return gti;
  graph_train_info new_gti;
  new_gti._times_valid = gti._times_valid;

  _rts._stats._ops.updater.extract_route++;

  // create new route id
  uint32_t new_route_id = ++_rts._max_route_id;

  motis::light_connection* start_lc = gti._edges[0]._lc;
  motis::node* old_route_start = gti._edges[0]._from_route_node;
  const int day_index = start_lc->d_time / MINUTES_A_DAY;
  std::unordered_set<uint32_t> train_nrs;

  if (_rts.is_debug_mode()) {
    LOG(debug) << "moving connection into new route: "
               << "new_route_id=" << new_route_id
               << ", start node: route_id=" << old_route_start->_route
               << ", station_index=" << old_route_start->get_station()->_id
               << ", start_lc: " << format_time(start_lc->d_time) << " -> "
               << format_time(start_lc->a_time) << " (" << start_lc << ")"
               << ", train_nr=" << start_lc->_full_con->con_info->train_nr;
    dump_route(old_route_start, "original old route", day_index);
  }

  assert(check_route(old_route_start));

  motis::node* new_route_start = nullptr;
  motis::node* new_from_route_node = nullptr;

  for (const graph_train_edge& e : gti._edges) {
    int32_t old_route_id = e._from_route_node->_route;
    // create route nodes
    if (new_from_route_node == nullptr ||
        new_from_route_node->get_station() !=
            e._from_route_node->get_station()) {
      new_from_route_node = copy_route_node(e._from_route_node, new_route_id);
      if (new_route_start == nullptr) new_route_start = new_from_route_node;
    }
    motis::edge* old_edge = _rts.get_next_edge(e._from_route_node);
    assert(old_edge != nullptr);
    motis::node* new_to_route_node =
        copy_route_node(old_edge->_to, new_route_id);

    // create edge
    // new_to_route_node->_previous_route_node = new_from_route_node;
    motis::light_connection lc = *e._lc;
    motis::connection* con = new connection(*lc._full_con);
    lc._full_con = con;
    _rts._new_full_connections.emplace_back(con);
    new_from_route_node->_edges.emplace_back(
        make_route_edge(new_from_route_node, new_to_route_node, {lc}));
    // update_incoming_edge(new_from_route_node);
    train_nrs.insert(con->con_info->train_nr);

    // update delay_infos
    motis::station_node* departure_station = e._from_route_node->get_station();
    motis::station_node* arrival_station = old_edge->_to->get_station();
    assert(departure_station != nullptr && arrival_station != nullptr);
    graph_event old_departure(departure_station->_id,
                              lc._full_con->con_info->train_nr, true, lc.d_time,
                              old_route_id);
    graph_event old_arrival(arrival_station->_id,
                            lc._full_con->con_info->train_nr, false, lc.a_time,
                            old_route_id);

    // update delay_info._route_id for both events (departure + arrival)
    delay_info* dep_di = _rts._delay_info_manager.get_delay_info(old_departure);
    if (dep_di != nullptr) {
      if (_rts.is_debug_mode())
        LOG(debug) << "    updating departure DI route_id " << *dep_di << " -> "
                   << new_route_id;
      _rts._delay_info_manager.update_route(dep_di, new_route_id);
    } else if (_rts.is_debug_mode()) {
      LOG(debug) << "    no departure DI for " << old_departure;
    }

    delay_info* arr_di = _rts._delay_info_manager.get_delay_info(old_arrival);
    if (arr_di != nullptr) {
      if (_rts.is_debug_mode())
        LOG(debug) << "    updating arrival DI route_id " << *arr_di << " -> "
                   << new_route_id;
      _rts._delay_info_manager.update_route(arr_di, new_route_id);
    } else if (_rts.is_debug_mode()) {
      LOG(debug) << "    no arrival DI for " << old_arrival;
    }

    if (lc.a_time < lc.d_time && _rts.is_debug_mode()) {
      LOG(warn) << "(temp) corrupt LC: d_time=" << format_time(lc.d_time)
                << ", a_time=" << format_time(lc.a_time);
    }
    assert(lc.a_time >= lc.d_time);

    // remove old LC
    old_edge->_m._route_edge._conns.erase(e._lc);

    // copy waiting edges
    _rts._waiting_edges.event_moved_to_new_route(
        _rts.get_schedule_event(old_departure), old_route_id, new_route_id);
    _rts._waiting_edges.event_moved_to_new_route(
        _rts.get_schedule_event(old_arrival), old_route_id, new_route_id);

    new_gti._edges.emplace_back(
        new_from_route_node,
        &_rts.get_next_edge(new_from_route_node)->_m._route_edge._conns.back(),
        e._dep_update, e._arr_update);

    new_from_route_node = new_to_route_node;
  }

  assert(new_route_start != nullptr);
  add_incoming_edges(new_route_start);

  _rts._schedule.route_index_to_first_route_node[new_route_id] =
      new_route_start;

  for (auto train_nr : train_nrs) {
    _rts._schedule.train_nr_to_routes[train_nr].push_back(new_route_id);
  }

  if (_rts.is_debug_mode()) {
    dump_route(old_route_start, "updated old route", day_index);
    dump_route(new_route_start, "new route", day_index);
  }

  assert(check_route(old_route_start));
  assert(check_route(new_route_start));

  return new_gti;
}

modified_train* graph_updater::make_modified_train(
    motis::node* start_route_node, motis::light_connection* start_lc) {
  motis::node* route_node = start_route_node;
  motis::light_connection* lc = start_lc;
  motis::edge* route_edge = _rts.get_next_edge(start_route_node);

  _rts._stats._ops.updater.make_modified++;

  //  assert(_rts.get_prev_node(start_route_node) == nullptr);
  assert(check_route(start_route_node, true));

  int32_t old_route_id = start_route_node->_route;
  bool new_route = !_rts.is_single_train_route(start_route_node);
  int32_t new_route_id = new_route ? ++_rts._max_route_id : old_route_id;

  if (_rts.is_debug_mode()) {
    LOG(debug) << "make_modified_train: old_route_id=" << old_route_id
               << ", new_route=" << new_route
               << ", new_route_id=" << new_route_id;
  }

  motis::connection_info* ci = new connection_info(*lc->_full_con->con_info);
  _rts._new_connection_infos.emplace_back(ci);

  modified_train* mt =
      new modified_train(old_route_id, new_route_id, ci, lc->_full_con->clasz);

  while (route_edge != nullptr && lc != nullptr) {
    motis::node* next_route_node = route_edge->_to;
    assert(next_route_node != nullptr);
    motis::edge* next_route_edge = _rts.get_next_edge(next_route_node);
    motis::light_connection* next_lc =
        next_route_edge == nullptr
            ? nullptr
            : next_route_edge->get_connection(lc->a_time);

    motis::station_node* departure_station = route_node->get_station();
    graph_event departure(departure_station->_id,
                          lc->_full_con->con_info->train_nr, true, lc->d_time,
                          old_route_id);
    schedule_event original_departure = _rts.get_schedule_event(departure);
    delay_info* dep_di =
        _rts._delay_info_manager.get_delay_info(original_departure);

    motis::station_node* arrival_station = next_route_node->get_station();
    graph_event arrival(arrival_station->_id, lc->_full_con->con_info->train_nr,
                        false, lc->a_time, old_route_id);
    schedule_event original_arrival = _rts.get_schedule_event(arrival);
    delay_info* arr_di =
        _rts._delay_info_manager.get_delay_info(original_arrival);

    const motis::connection* full_con = lc->_full_con;

    mt->_original_events.emplace_back(original_departure, full_con,
                                      has_entering_edge(route_node),
                                      has_leaving_edge(route_node));
    mt->_original_events.emplace_back(original_arrival, full_con,
                                      has_entering_edge(next_route_node),
                                      has_leaving_edge(next_route_node));

    if (new_route) {
      route_edge->_m._route_edge._conns.erase(lc);
      _rts._waiting_edges.event_moved_to_new_route(original_departure,
                                                   old_route_id, new_route_id);
      _rts._waiting_edges.event_moved_to_new_route(original_arrival,
                                                   old_route_id, new_route_id);
      if (dep_di != nullptr) {
        _rts._delay_info_manager.update_route(dep_di, new_route_id);
      }
      if (arr_di != nullptr) {
        _rts._delay_info_manager.update_route(arr_di, new_route_id);
      }
    } else {
      // create new connection so that we can modify it later
      motis::connection* connection_copy = new connection(*lc->_full_con);
      _rts._new_full_connections.emplace_back(connection_copy);
      lc->_full_con = connection_copy;
      if (!mt->_current_start_event.found()) {
        mt->_current_start_event = original_departure;
      }
    }

    route_node = next_route_node;
    lc = next_lc;
    route_edge = next_route_edge;
  }

  assert(check_route(start_route_node, true));

  if (new_route) {
    _rts._schedule.train_nr_to_routes[mt->_connection_info->train_nr].push_back(
        new_route_id);
  }

  _rts._modified_train_manager.add(mt);
  return mt;
}

void graph_updater::adjust_train(modified_train* mt,
                                 std::vector<schedule_event> new_events,
                                 bool update_all_edges) {
  operation_timer timer(_rts._stats._graph_updater);
  if (_rts.is_debug_mode())
    LOG(debug) << "adjust_train: route_id=" << mt->_new_route_id << ", "
               << new_events.size() << " new events";
  _rts._stats._ops.updater.adjust_train++;

  std::vector<std::tuple<motis::node*, schedule_event, schedule_event>>
      current_events = _rts.get_train_events(mt->_current_start_event);

  if (_rts.is_debug_mode()) {
    LOG(debug) << "current_events (current_start_event="
               << mt->_current_start_event << "):";
    for (auto& e : current_events) {
      motis::node* route_node;
      schedule_event arr, dep;
      std::tie(route_node, arr, dep) = e;
      LOG(debug) << "-- " << route_node->get_station()->_id << ", a: " << arr
                 << ", d: " << dep;
    }
    if (!current_events.empty())
      dump_route(std::get<0>(current_events[0]), "route before adjust");
  }

  if (!current_events.empty()) {
    motis::node* start_node = std::get<0>(current_events[0]);
    assert(check_route(start_node));
    remove_incoming_edges(start_node);
  }

  schedule_event departure_event, arrival_event;
  motis::node* route_node = nullptr;
  motis::node* last_route_node = nullptr;
  motis::node* start_node = nullptr;
  schedule_event current_start_event;

  for (std::size_t ni = 0; ni < new_events.size(); ni++) {
    schedule_event& new_event = new_events[ni];

    if (_rts.is_debug_mode())
      LOG(debug) << "new_events[" << ni << "] = " << new_event
                 << ", departure_event = " << departure_event
                 << ", arrival_event = " << arrival_event;

    // get or create route node
    if (route_node == nullptr ||
        arrival_event._station_index != new_event._station_index) {
      bool enter = ni != new_events.size() - 1;
      bool leave = ni != 0;
      const original_train_event* original_event =
          mt->original_event_at_station(new_event._station_index);
      if (original_event != nullptr) {
        enter = original_event->_entering_allowed;
        leave = original_event->_leaving_allowed;
      }
      auto it = std::find_if(
          std::begin(current_events), std::end(current_events),
          [&](const std::tuple<node*, schedule_event, schedule_event>& p) {
            return std::get<1>(p)._station_index == new_event._station_index ||
                   std::get<2>(p)._station_index == new_event._station_index;
          });
      if (it != std::end(current_events)) {
        route_node = std::get<0>(*it);
        if (_rts.is_debug_mode())
          LOG(debug)
              << "  using existing route node @"
              << _rts._schedule.stations[route_node->get_station()->_id]->name;
        fix_foot_edges(route_node, enter, leave);
        current_events.erase(it);
      } else {
        if (_rts.is_debug_mode())
          LOG(debug) << "  creating new route node @"
                     << _rts._schedule.stations[new_event._station_index]->name;
        route_node = create_route_node(new_event._station_index,
                                       mt->_new_route_id, enter, leave);
      }
    }

    // make sure the route edge is correct
    if (new_event.arrival()) {
      assert(last_route_node != nullptr);
      motis::edge* route_edge = _rts.get_next_edge(last_route_node);
      // reset all times to schedule time - events will be added to delay
      // propagator
      // to recalculate the new times
      motis::time departure_time =
          _rts._delay_info_manager.reset_to_schedule(departure_event);
      motis::time arrival_time =
          _rts._delay_info_manager.reset_to_schedule(new_event);
      if (_rts.is_debug_mode())
        LOG(debug)
            << "  creating/updating route edge from "
            << _rts._schedule.stations[last_route_node->get_station()->_id]
                   ->name << " [" << format_time(departure_time) << "]"
            << " to "
            << _rts._schedule.stations[route_node->get_station()->_id]->name
            << " [" << format_time(arrival_time) << "]";
      bool update_full_con = update_all_edges;
      motis::connection* con = nullptr;
      if (route_edge == nullptr) {
        // create new route edge
        con = new motis::connection();
        con->con_info = mt->_connection_info;
        con->clasz = mt->_clasz;
        update_full_con = true;
        _rts._new_full_connections.emplace_back(con);
        std::vector<light_connection> lcs;
        lcs.emplace_back(departure_time, arrival_time, con);
        last_route_node->_edges.emplace_back(
            make_route_edge(last_route_node, route_node, lcs));
        if (_rts.is_debug_mode()) LOG(debug) << "    created new route edge";
      } else {
        // modify route edge
        if (route_edge->_to != route_node) {
          // route_edge->_to->_previous_route_node = nullptr;
          // remove_incoming_route_edge(route_edge->_to); // TODO?
          route_edge->_to = route_node;
          update_full_con = true;
          if (_rts.is_debug_mode())
            LOG(debug) << "    modified existing route edge";
        } else if (_rts.is_debug_mode()) {
          LOG(debug) << "    used existing route edge";
        }
        assert(route_edge->_m._route_edge._conns.size() == 1);
        light_connection& lc = route_edge->_m._route_edge._conns[0];
        lc.d_time = departure_time;
        lc.a_time = arrival_time;
        if (update_full_con) {
          // this connection object is only used for this single
          // light_connection, so we can safely modify it without
          // affecting other trains.
          con = const_cast<connection*>(lc._full_con);
          con->con_info = mt->_connection_info;
          con->clasz = mt->_clasz;
        }
      }
      if (update_full_con) {
        con->price =
            get_distance(
                *_rts._schedule.stations[departure_event._station_index].get(),
                *_rts._schedule.stations[new_event._station_index].get()) *
            get_price_per_km(mt->_clasz);
      }
    }

    // route_node->_previous_route_node = last_route_node;
    // if (last_route_node != nullptr) {
    //   update_incoming_edge(last_route_node);
    // } else {
    //   remove_incoming_route_edge(route_node);
    // }
    assert(route_node != last_route_node);
    if (new_event.departure()) {
      last_route_node = route_node;
      departure_event = new_event;
    } else {
      arrival_event = new_event;
    }

    if (ni == 0) {
      start_node = route_node;
      current_start_event = new_event;
      assert(current_start_event.found());
    }
  }

  // make sure that there is no outgoing route edge at the last node
  if (route_node != nullptr) {
    motis::edge* route_edge = _rts.get_next_edge(route_node);
    if (route_edge != nullptr) {
      if (_rts.is_debug_mode())
        LOG(debug) << "removing route edge at last stop";
      route_node->_edges.erase(route_edge);
    }
  }

  // remove route nodes that are no longer needed
  for (auto& p : current_events) {
    node* route_node = std::get<0>(p);
    if (_rts.is_debug_mode())
      LOG(debug)
          << "removing route node @"
          << _rts._schedule.stations[route_node->get_station()->_id]->name;
    delete_route_node(route_node);
  }

  mt->_current_start_event = current_start_event;
  _rts._schedule.route_index_to_first_route_node[mt->_new_route_id] =
      start_node;
  if (start_node != nullptr) {
    add_incoming_edges(start_node);
    if (_rts.is_debug_mode()) {
      graph_event ge = _rts.get_graph_event(current_start_event);
      LOG(debug) << "new MT start event: " << current_start_event << " -> "
                 << ge;
      dump_route(start_node, "route after adjust");

      motis::node* n;
      motis::light_connection* l;
      std::tie(n, l) = _rts.locate_event(ge);
      if (n == nullptr) {
        LOG(error) << "event not found: " << ge;
      } else if (n != start_node) {
        LOG(error) << "wrong node found for event: " << ge;
        dump_route(n, "route that was found");
        LOG(error) << "LC: " << format_time(l->d_time) << "->"
                   << format_time(l->a_time)
                   << ", train_nr=" << l->_full_con->con_info->train_nr;
      }
      assert(check_route(start_node));
      assert(n == start_node);
    }
  }
  if (_rts.is_debug_mode()) LOG(debug) << "adjust_train done";
}

void graph_updater::finish_graph_update() {
  _rts._schedule.lower_bounds = constant_graph(_rts._schedule.station_nodes);
}

node* graph_updater::copy_route_node(motis::node* original_node,
                                     uint32_t new_route_id) {
  motis::station_node* station = original_node->get_station();
  motis::node* new_node = new node(station, _rts._schedule.node_count++);
  new_node->_route = new_route_id;

  //  LOG(debug) << "copy_route_node @station " << station->_id
  //             << " (" << station << ")"
  //             << ", route_id=" << original_node->_route
  //             << ", new_route_id=" << new_route_id;

  remove_incoming_edges(station);

  // copy outgoing edges (except for route edge)
  for (const motis::edge& original_edge : original_node->_edges) {

    if (original_edge.get_destination() == station) {
      if (original_edge.type() == edge::FOOT_EDGE) {
        //        LOG(debug) << "  copy outgoing foot edge";
        new_node->_edges.push_back(make_foot_edge(
            new_node, station, original_edge._m._foot_edge._time_cost,
            original_edge._m._foot_edge._transfer));
      } else if (original_edge.type() == edge::INVALID_EDGE) {
        //        LOG(debug) << "  copy outgoing invalid edge";
        new_node->_edges.push_back(make_invalid_edge(new_node, station));
      }
    } else if (original_edge.type() == edge::AFTER_TRAIN_FOOT_EDGE &&
               original_edge.get_destination() == station->_foot_node) {
      new_node->_edges.push_back(make_after_train_edge(
          new_node, station->_foot_node, original_edge._m._foot_edge._time_cost,
          original_edge._m._foot_edge._transfer));
    }
  }

  // copy station->route node edge (either foot or invalid edge)
  for (unsigned i = 0; i < station->_edges.size(); i++) {
    const motis::edge& original_edge = station->_edges[i];
    if (original_edge.get_destination() == original_node) {
      //      LOG(debug) << "  copy incoming edge with type="
      //                 << static_cast<int>(original_edge.type());
      if (original_edge.type() == edge::FOOT_EDGE)
        station->_edges.push_back(make_foot_edge(
            station, new_node, original_edge._m._foot_edge._time_cost,
            original_edge._m._foot_edge._transfer));
      else if (original_edge.type() == edge::INVALID_EDGE)
        station->_edges.push_back(make_invalid_edge(station, new_node));
    }
  }

  add_incoming_edges(station);

  return new_node;
}

node* graph_updater::create_route_node(unsigned station_index, int32_t route_id,
                                       bool enter, bool leave) {
  motis::station_node* station =
      _rts._schedule.station_nodes[station_index].get();
  motis::node* route_node = new node(station, _rts._schedule.node_count++);
  route_node->_route = route_id;

  remove_incoming_edges(station);

  if (leave) {
    route_node->_edges.emplace_back(make_foot_edge(
        route_node, station,
        _rts._schedule.stations[station_index]->transfer_time, true));
    if (station->_foot_node != nullptr) {
      route_node->_edges.emplace_back(
          make_after_train_edge(route_node, station->_foot_node, 0, true));
    }
  }
  // route_node->_previous_route_node = nullptr;

  if (enter)
    station->_edges.push_back(make_foot_edge(station, route_node));
  else
    station->_edges.push_back(make_invalid_edge(station, route_node));

  if (_rts.is_debug_mode())
    LOG(debug) << "created route node " << route_node->_id << " at station "
               << station_index << ", route_id=" << route_id
               << ", enter=" << enter << ", leave=" << leave;

  add_incoming_edges(station);

  return route_node;
}

void graph_updater::delete_route_node(motis::node* route_node) {
  motis::station_node* station = route_node->get_station();
  remove_incoming_edges(station);
  for (motis::edge& edge : station->_edges) {
    if (edge.get_destination() == route_node) {
      station->_edges.erase(&edge);
      break;
    }
  }
  delete route_node;
  add_incoming_edges(station);
}

void graph_updater::fix_foot_edges(motis::node* route_node, bool enter,
                                   bool leave) {
  motis::station_node* station = route_node->get_station();

  auto leaving_edge =
      std::find_if(std::begin(route_node->_edges), std::end(route_node->_edges),
                   [&](edge& e) { return e.get_destination() == station; });
  if (leaving_edge == std::end(route_node->_edges)) {
    if (leave) {
      // add missing leaving edge
      route_node->_edges.emplace_back(make_foot_edge(
          route_node, station,
          _rts._schedule.stations[station->_id]->transfer_time, true));
      if (station->_foot_node != nullptr) {
        route_node->_edges.emplace_back(
            make_after_train_edge(route_node, station->_foot_node, 0, true));
      }
    }
  } else if (!leave) {
    // remove existing leaving edge
    route_node->_edges.erase(leaving_edge);
    if (station->_foot_node != nullptr) {
      for (motis::edge& edge : route_node->_edges) {
        if (edge.get_destination() == station->_foot_node) {
          route_node->_edges.erase(&edge);
          break;
        }
      }
    }
  }

  // set type of entering edge
  for (motis::edge& edge : station->_edges) {
    if (edge.get_destination() == route_node) {
      edge._m._type = enter ? edge::FOOT_EDGE : edge::INVALID_EDGE;
      break;
    }
  }
}

void graph_updater::remove_incoming_edges(motis::node* node) {
  if (node->is_route_node()) {
    node = _rts.get_start_node(node);
  }
  do {
    for (motis::edge& e : node->_edges) {
      motis::node* target = e._to;
      assert(e._from == node);
      for (auto& ie : target->_incoming_edges) {
        if (ie == &e) {
          target->_incoming_edges.erase(&ie);
          break;
        }
      }
    }
    node = node->is_route_node() ? _rts.get_next_node(node) : nullptr;
  } while (node != nullptr);
}

void graph_updater::add_incoming_edges(motis::node* node) {
  if (node->is_route_node()) {
    node = _rts.get_start_node(node);
  }
  do {
    for (motis::edge& e : node->_edges) {
      motis::node* target = e._to;
      e._from = node;
      target->_incoming_edges.push_back(&e);
    }
    node = node->is_route_node() ? _rts.get_next_node(node) : nullptr;
  } while (node != nullptr);
}

bool graph_updater::has_entering_edge(const motis::node* route_node) const {
  for (const motis::edge& edge : route_node->get_station()->_edges) {
    if (edge.get_destination() == route_node &&
        edge.type() == edge::FOOT_EDGE) {
      return true;
    }
  }
  return false;
}

bool graph_updater::has_leaving_edge(const motis::node* route_node) const {
  const station_node* station_node = route_node->get_station();
  for (const motis::edge& edge : route_node->_edges) {
    if (edge.get_destination() == station_node &&
        edge.type() == edge::FOOT_EDGE) {
      return true;
    }
  }
  return false;
}

bool graph_updater::check_route(motis::node* ref_node,
                                bool require_times) const {
  motis::node* start_node = ref_node;
  while (_rts.get_prev_node(start_node) != nullptr)
    start_node = _rts.get_prev_node(start_node);

  motis::node* last_node = start_node;
  motis::edge* last_edge = _rts.get_next_edge(last_node);
  motis::node* current_node = last_edge->_to;
  motis::edge* current_edge = _rts.get_next_edge(current_node);

  while (current_edge != nullptr) {
    if (last_node->_route != current_node->_route) {
      LOG(error) << "route corrupt: route id changes from " << last_node->_route
                 << " to " << current_node->_route;
      dump_route(start_node, "corrupt route", -1);
      return false;
    }

    for (uint32_t i = 0; i < last_edge->_m._route_edge._conns.size(); i++) {
      const motis::light_connection* lc1 = &last_edge->_m._route_edge._conns[i];
      const motis::light_connection* lc2 =
          current_edge->get_connection(lc1->a_time);
      if (lc2 == nullptr) continue;
      if (require_times || last_edge->_m._route_edge._conns.size() > 1) {
        if (lc1->a_time < lc1->d_time) {
          LOG(error) << "route corrupt: invalid times in LC: "
                     << motis::format_time(lc1->d_time) << "->"
                     << motis::format_time(lc1->a_time) << " ["
                     << lc1->_full_con->con_info->train_nr << "]";
          dump_route(start_node, "corrupt route", lc1->d_time / MINUTES_A_DAY);
          return false;
        }
        if (lc2->a_time < lc2->d_time) {
          LOG(error) << "route corrupt: invalid times in LC: "
                     << motis::format_time(lc2->d_time) << "->"
                     << motis::format_time(lc2->a_time) << " ["
                     << lc2->_full_con->con_info->train_nr << "]";
          dump_route(start_node, "corrupt route", lc1->d_time / MINUTES_A_DAY);
          return false;
        }
        if (lc2->d_time < lc1->a_time) {
          LOG(error) << "route corrupt: invalid times in LCs: "
                     << motis::format_time(lc1->d_time) << "->"
                     << motis::format_time(lc1->a_time) << " ["
                     << lc1->_full_con->con_info->train_nr << "] and "
                     << motis::format_time(lc2->d_time) << "->"
                     << motis::format_time(lc2->a_time) << " ["
                     << lc2->_full_con->con_info->train_nr << "]";
          dump_route(start_node, "corrupt route", lc1->d_time / MINUTES_A_DAY);
          return false;
        }
      }
    }

    last_node = current_node;
    last_edge = current_edge;
    current_node = last_edge->_to;
    current_edge =
        current_node == nullptr ? nullptr : _rts.get_next_edge(current_node);
  }

  return true;
}

// DEBUG
void graph_updater::dump_route(motis::node* start_node, const char* title,
                               int day_index) const {
  assert(start_node != nullptr);

  while (_rts.get_prev_node(start_node) != nullptr)
    start_node = _rts.get_prev_node(start_node);

  LOG(debug) << "-------- route: " << title << " (" << start_node->_route
             << ")";

  motis::node* route_node = start_node;
  while (route_node != nullptr) {
    motis::edge* route_edge = _rts.get_next_edge(route_node);
    if (route_edge == nullptr) break;
    motis::node* next_route_node = route_edge->_to;
    assert(next_route_node != nullptr);

    motis::station_node* dep_station = route_node->get_station();
    motis::station_node* arr_station = next_route_node->get_station();
    assert(dep_station != nullptr);
    assert(arr_station != nullptr);

    int enter = -1;
    int leave = -1;
    for (edge& e : dep_station->_edges) {
      if (e._to == route_node) enter = e.type() == edge::FOOT_EDGE ? 1 : 0;
    }
    for (edge& e : next_route_node->_edges) {
      if (e._to == arr_station) leave = e.type() == edge::FOOT_EDGE ? 1 : 0;
    }

    std::ostream& os =
        LOG(debug) << route_node << ": "
                   << _rts._schedule.stations[dep_station->_id]->name << " ("
                   << dep_station->_id << ", enter=" << enter << ") -> "
                   << _rts._schedule.stations[arr_station->_id]->name << " ("
                   << arr_station->_id << ", leave=" << leave
                   << "): " << route_edge->_m._route_edge._conns.size() << "c ";

    for (light_connection& lc : route_edge->_m._route_edge._conns) {
      if (route_edge->_m._route_edge._conns.size() > 4 && day_index != -1 &&
          (lc.d_time / MINUTES_A_DAY != day_index &&
           lc.a_time / MINUTES_A_DAY != day_index))
        continue;
      os << "  " << motis::format_time(lc.d_time) << "->"
         << motis::format_time(lc.a_time) << "["
         << lc._full_con->con_info->train_nr << "]";
    }

    route_node = next_route_node;
  }

  LOG(debug) << "--------";
}

bool graph_updater::check_updates(
    std::vector<delay_info_update>& updates) const {
  motis::time t = 0;
  for (const auto& u : updates) {
    if (u._new_time < t) return false;
    t = u._new_time;
  }
  return true;
}

}  // namespace realtime
}  // namespace motis
