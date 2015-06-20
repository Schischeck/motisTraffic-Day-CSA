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
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/station.h"
#include "motis/core/schedule/edges.h"
#include "motis/routing/lower_bounds.h"
#include "motis/routing/pareto_dijkstra.h"
#include "motis/routing/label.h"
#include "motis/routing/timing.h"

namespace motis {

void remove_intersection(arrival& from, arrival& to) {
  for (auto const& to_arr_part : to)
    from.erase(
        std::remove_if(std::begin(from), std::end(from),
                       [&to_arr_part](arrival_part const& from_arr_part) {
          return to_arr_part.station == from_arr_part.station;
        }),
        std::end(from));
}

search::search(schedule& schedule, memory_manager<label>& label_store)
    : _sched(schedule), _label_store(label_store) {}

std::vector<journey> search::get_connections(
    arrival from, arrival to, int time1, int time2, int day,
    pareto_dijkstra::statistics* stats) {
  _label_store.reset();
  remove_intersection(from, to);

  // use dummy station as virtual station representing
  // all source stations as well as all target stations
  assert(!_sched.stations.empty());
  int dummy_source = _sched.stations.front()->index;
  int dummy_target = _sched.stations.back()->index;

  // generate additional edges for the lower bound graph.
  std::unordered_map<int, std::vector<simple_edge>> lb_graph_edges;
  for (auto const& arr : to)
    lb_graph_edges[dummy_target].push_back(
        simple_edge(arr.station, arr.time_cost));
  for (auto const& arr : from)
    lb_graph_edges[arr.station].push_back(
        simple_edge(dummy_source, arr.time_cost));

  // initialize lower bound graphs and
  // check if there is a path from source to target

  lower_bounds lower_bounds(_sched.lower_bounds, dummy_target, lb_graph_edges);

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

  if (lower_bounds.travel_time.get_distance(dummy_source) ==
      std::numeric_limits<uint32_t>::max()) {
    LOG(logging::error) << "no path from source[" << dummy_source << "] "
                        << "to target[" << dummy_target << "] found";
    return {};
  }

  int day2 = day;
  if (time1 > time2) ++day2;

  std::vector<label*> start_labels;
  station_node* dummy_source_station = _sched.station_nodes[dummy_source].get();
  for (const auto& s : from) {
    station_node const* station = _sched.station_nodes[s.station].get();

    // generate labels at all route nodes
    // for all trains departing in the specified interval
    generate_start_labels(to_time(day, time1), to_time(day2, time2), station,
                          start_labels, dummy_source_station, s.time_cost,
                          s.price, s.slot, lower_bounds);
  }

  std::unordered_map<node const*, std::vector<edge>> additional_edges;
  station_node* target = _sched.station_nodes[dummy_target].get();
  for (auto const& arr : to) {
    station_node* arrival_station = _sched.station_nodes[arr.station].get();

    auto it = additional_edges.find(arrival_station);
    if (it == end(additional_edges))
      std::tie(it, std::ignore) =
          additional_edges.emplace(arrival_station, std::vector<edge>());

    it->second.emplace_back(
        make_mumo_edge(target, arr.time_cost, arr.price, arr.slot));
  }

  pareto_dijkstra pd(_sched.node_count,
                     _sched.station_nodes[dummy_target].get(), start_labels,
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
  std::transform(std::begin(results), std::end(results), std::begin(journeys),
                 [this](label* label) { return journey(label, _sched); });

  return journeys;
}

void search::generate_start_labels(time const from, time const to,
                                   station_node const* start_station_node,
                                   std::vector<label*>& indices,
                                   station_node const* real_start, int time_off,
                                   int start_price, int slot,
                                   lower_bounds& lower_bounds) {
  for (auto const& edge : start_station_node->_edges)
    generate_start_labels(from, to, start_station_node, edge.get_destination(),
                          indices, real_start, time_off, start_price, slot,
                          lower_bounds);
}

void search::generate_start_labels(time const from, time const to,
                                   station_node const* start_station_node,
                                   node const* route_node,
                                   std::vector<label*>& indices,
                                   station_node const* real_start, int time_off,
                                   int start_price, int slot,
                                   lower_bounds& lower_bounds) {
  for (auto const& edge : route_node->_edges) {
    // not a route-edge?
    if (edge.get_destination() == start_station_node) continue;

    time t = from + time_off;

    // don't set label on foot node
    // this isn't neccesary in a intermodal scenario.
    if (edge._m._type != edge::type::ROUTE_EDGE) {
      continue;
    }

    while (t <= to + time_off) {
      light_connection const* con = edge.get_connection(t);
      if (con == nullptr) break;

      t = con->d_time;

      if (t > to + time_off) break;

      // was there an earlier start station?
      label* earlier = nullptr;
      if (real_start != nullptr)
        earlier = new (_label_store.create())
            label(real_start, nullptr, t - time_off, lower_bounds);

      label* station_node_label = new (_label_store.create())
          label(start_station_node, earlier, t, lower_bounds);
      station_node_label->_prices[label::ADDITIONAL_PRICE] = start_price;
      station_node_label->_total_price[0] = start_price;

      // create the label we are really interested in
      label* route_node_label = new (_label_store.create())
          label(route_node, station_node_label, t, lower_bounds);
      route_node_label->set_slot(true, slot);

      indices.push_back(route_node_label);

      t = t + 1;
    }
  }
}

}  // namespace motis
