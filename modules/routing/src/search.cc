#include "motis/routing/search.h"

#include <cassert>
#include <ctime>
#include <iostream>
#include <string>
#include <algorithm>
#include <list>
#include <set>
#include <stdexcept>
#include <iterator>
#include <unordered_map>

#include "boost/lexical_cast.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/timing.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/station.h"
#include "motis/core/schedule/edges.h"
#include "motis/routing/label.h"
#include "motis/routing/labels_to_journey.h"
#include "motis/routing/lower_bounds.h"
#include "motis/routing/pareto_dijkstra.h"

namespace motis {

void remove_intersection(arrival& from, arrival& to) {
  for (auto const& to_arr_part : to) {
    from.erase(
        std::remove_if(std::begin(from), std::end(from),
                       [&to_arr_part](arrival_part const& from_arr_part) {
                         return to_arr_part.station == from_arr_part.station;
                       }),
        std::end(from));
  }
}

search::search(schedule const& schedule, memory_manager<label>& label_store)
    : _sched(schedule), _label_store(label_store) {}

void add_additional_edge(
    node const* n, edge e,
    std::unordered_map<node const*, std::vector<edge>>& additional_edges) {
  auto it = additional_edges.find(n);
  if (it == end(additional_edges)) {
    std::tie(it, std::ignore) =
        additional_edges.emplace(n, std::vector<edge>());
  }
  it->second.emplace_back(e);
}

std::vector<journey> search::get_connections(
    arrival from, arrival to, time interval_start, time interval_end,
    bool ontrip, bool is_late_connection_search,
    std::vector<edge> const& query_additional_edges,
    pareto_dijkstra::statistics* stats) {
  _label_store.reset();
  remove_intersection(from, to);

  // use dummy station as virtual station representing
  // all source stations as well as all target stations
  assert(!_sched.stations.empty());

  // generate additional edges for the lower bound graph.
  std::unordered_map<int, std::vector<simple_edge>> lb_graph_edges;
  for (auto const& arr : to) {
    lb_graph_edges[DUMMY_TARGET_IDX].push_back(
        simple_edge(arr.station, arr.time_cost));
  }
  for (auto const& arr : from) {
    lb_graph_edges[arr.station].push_back(
        simple_edge(DUMMY_SOURCE_IDX, arr.time_cost));
  }
  for (auto const& e : query_additional_edges) {
    lb_graph_edges[e._to->get_station()->_id].emplace_back(
        e._from->get_station()->_id, e.get_minimum_cost());
  }

  // initialize lower bound graphs and
  // check if there is a path from source to target
  lower_bounds lower_bounds(_sched.lower_bounds, DUMMY_TARGET_IDX,
                            lb_graph_edges);

  MOTIS_START_TIMING(lower_bounds_timing);

  MOTIS_START_TIMING(travel_l_b_time_timing);
  lower_bounds.travel_time.run();
  MOTIS_STOP_TIMING(travel_l_b_time_timing);

  MOTIS_START_TIMING(transfers_l_b_timing);
  lower_bounds.transfers.run();
  MOTIS_STOP_TIMING(transfers_l_b_timing);

  MOTIS_START_TIMING(price_l_b_timing);
  lower_bounds.price.run();
  MOTIS_STOP_TIMING(price_l_b_timing);

  MOTIS_STOP_TIMING(lower_bounds_timing);

  if (lower_bounds.travel_time.get_distance(DUMMY_SOURCE_IDX) ==
      std::numeric_limits<uint32_t>::max()) {
    LOG(logging::error) << "no path from source to target found";
    return {};
  }

  std::vector<label*> start_labels;
  if (ontrip) {
    if (from.size() != 1) {
      throw std::runtime_error("ontrip accepts exactly one station");
    }
    generate_ontrip_start_labels(_sched.station_nodes[from[0].station].get(),
                                 interval_start, start_labels, lower_bounds,
                                 is_late_connection_search);
  } else {
    station_node* dummy_source_station =
        _sched.station_nodes[DUMMY_SOURCE_IDX].get();
    for (const auto& s : from) {
      station_node const* station = _sched.station_nodes[s.station].get();

      // generate labels at all route nodes
      // for all trains departing in the specified interval
      generate_start_labels(
          interval_start, interval_end, station, start_labels,
          dummy_source_station,
          s.time_cost + _sched.stations[s.station]->transfer_time, s.price,
          s.slot, lower_bounds, is_late_connection_search);
    }
  }

  std::unordered_map<node const*, std::vector<edge>> additional_edges;
  station_node* target = _sched.station_nodes[DUMMY_TARGET_IDX].get();
  for (auto const& arr : to) {
    station_node* arrival_station = _sched.station_nodes[arr.station].get();
    add_additional_edge(arrival_station,
                        make_mumo_edge(arrival_station, target, arr.time_cost,
                                       arr.price, arr.slot),
                        additional_edges);
  }

  for (auto const& e : query_additional_edges) {
    add_additional_edge(e._from, e, additional_edges);
  }

  pareto_dijkstra pd(_sched.node_count,
                     _sched.station_nodes[DUMMY_TARGET_IDX].get(), start_labels,
                     additional_edges, lower_bounds, _label_store);
  std::vector<label*>& results = pd.search();

  if (stats != nullptr) {
    *stats = pd.get_statistics();
    stats->travel_time_l_b = MOTIS_TIMING_MS(travel_l_b_time_timing);
    stats->transfers_l_b = MOTIS_TIMING_MS(transfers_l_b_timing);
    stats->price_l_b = MOTIS_TIMING_MS(price_l_b_timing);
  }

  std::vector<journey> journeys;
  journeys.resize(results.size());
  std::transform(
      begin(results), end(results), begin(journeys),
      [this](label* label) { return labels_to_journey(label, _sched); });

  return journeys;
}

void search::generate_ontrip_start_labels(
    station_node const* start_station_node, time const start_time,
    std::vector<label*>& start_labels, lower_bounds& lower_bounds,
    bool const is_late_connection_search) {
  start_labels.push_back(new (_label_store.create())
                             label(start_station_node, nullptr, start_time,
                                   lower_bounds, is_late_connection_search));
}

void search::generate_start_labels(time const from, time const to,
                                   station_node const* start_station_node,
                                   std::vector<label*>& indices,
                                   station_node const* real_start, int time_off,
                                   int start_price, int slot,
                                   lower_bounds& lower_bounds,
                                   bool is_late_connection_search) {
  for (auto const& edge : start_station_node->_edges) {
    generate_start_labels(from, to, start_station_node, edge.get_destination(),
                          indices, real_start, time_off, start_price, slot,
                          lower_bounds, is_late_connection_search);
  }
}

void search::generate_start_labels(
    time const from, time const to, station_node const* start_station_node,
    node const* route_node, std::vector<label*>& start_labels,
    station_node const* real_start, int time_off, int start_price, int slot,
    lower_bounds& lower_bounds, bool const is_late_connection_search) {
  for (auto const& edge : route_node->_edges) {
    // not a route-edge?
    if (edge.empty()) {
      continue;
    }

    time t = from + time_off;

    // don't set label on foot node
    // this isn't neccesary in a intermodal scenario.
    if (edge._m._type != edge::ROUTE_EDGE) {
      continue;
    }

    while (t <= to + time_off) {
      light_connection const* con = edge.get_connection(t);
      if (con == nullptr) {
        break;
      }

      t = con->d_time;

      if (t > to + time_off) {
        break;
      }

      // was there an earlier start station?
      label* earlier = nullptr;
      if (real_start != nullptr) {
        earlier = new (_label_store.create())
            label(real_start, nullptr, t - time_off, lower_bounds,
                  is_late_connection_search);
      }

      label* station_node_label = new (_label_store.create())
          label(start_station_node, earlier, t, lower_bounds,
                is_late_connection_search);
      station_node_label->_prices[label::ADDITIONAL_PRICE] = start_price;
      station_node_label->_total_price[0] = start_price;

      // create the label we are really interested in
      label* route_node_label = new (_label_store.create())
          label(route_node, station_node_label, t, lower_bounds,
                is_late_connection_search);
      route_node_label->set_slot(true, slot);

      if (route_node_label->_travel_time[1] !=
          std::numeric_limits<uint16_t>::max()) {
        start_labels.push_back(route_node_label);
      }

      t = t + 1;
    }
  }
}

}  // namespace motis
