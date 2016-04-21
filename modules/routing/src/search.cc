#include "motis/routing/search.h"

#include <cassert>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <list>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "boost/lexical_cast.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/timing.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/station.h"
#include "motis/routing/lower_bounds.h"
#include "motis/routing/output/labels_to_journey.h"
#include "motis/routing/pareto_dijkstra.h"

namespace motis {
namespace routing {

void remove_intersection(arrival& from, arrival& to) {
  for (auto const& to_arr_part : to) {
    from.erase(
        std::remove_if(std::begin(from), std::end(from),
                       [&to_arr_part](arrival_part const& from_arr_part) {
                         return to_arr_part.station_ == from_arr_part.station_;
                       }),
        std::end(from));
  }
}

search::search(schedule const& schedule, memory_manager& label_store)
    : sched_(schedule), label_store_(label_store) {}

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

search_result search::get_connections(
    arrival from, arrival to, time interval_start, time interval_end,
    bool ontrip, std::vector<edge> const& query_additional_edges) {

  label_store_.reset();
  remove_intersection(from, to);

  // use dummy station as virtual station representing
  // all source stations as well as all target stations
  assert(!sched_.stations_.empty());

  // generate additional edges for the lower bound graph.
  std::unordered_map<int, std::vector<simple_edge>> lb_graph_edges;
  for (auto const& arr : to) {
    lb_graph_edges[DUMMY_TARGET_IDX].push_back(
        simple_edge(arr.station_, edge_cost(arr.time_cost_)));
  }
  for (auto const& arr : from) {
    lb_graph_edges[arr.station_].push_back(
        simple_edge(DUMMY_SOURCE_IDX, edge_cost(arr.time_cost_)));
  }
  for (auto const& e : query_additional_edges) {
    lb_graph_edges[e.to_->get_station()->id_].emplace_back(
        e.from_->get_station()->id_, e.get_minimum_cost());
  }

  // initialize lower bound graphs and
  // check if there is a path from source to target
  lower_bounds lb(sched_.lower_bounds_, DUMMY_TARGET_IDX, lb_graph_edges);
  lb.travel_time_.run();
  lb.transfers_.run();

  if (lb.travel_time_[DUMMY_SOURCE_IDX] ==
      std::numeric_limits<uint32_t>::max()) {
    LOG(logging::error) << "no path from source to target found";
    return search_result();
  }

  std::vector<my_label*> start_labels;
  if (ontrip) {
    if (from.size() != 1) {
      throw std::runtime_error("ontrip accepts exactly one station");
    }
    generate_ontrip_start_labels(sched_.station_nodes_[from[0].station_].get(),
                                 interval_start, start_labels, lb);
  } else {
    station_node* dummy_source_station =
        sched_.station_nodes_[DUMMY_SOURCE_IDX].get();
    for (const auto& s : from) {
      station_node const* station = sched_.station_nodes_[s.station_].get();

      // generate labels at all route nodes
      // for all trains departing in the specified interval
      generate_start_labels(interval_start, interval_end, station, start_labels,
                            dummy_source_station, s.time_cost_, lb);
    }
  }

  std::unordered_map<node const*, std::vector<edge>> additional_edges;
  station_node* target = sched_.station_nodes_[DUMMY_TARGET_IDX].get();
  for (auto const& arr : to) {
    auto arrival_station = sched_.station_nodes_[arr.station_].get();
    add_additional_edge(arrival_station,
                        make_mumo_edge(arrival_station, target, arr.time_cost_,
                                       arr.price_, arr.slot_),
                        additional_edges);
  }

  for (auto const& e : query_additional_edges) {
    add_additional_edge(e.from_, e, additional_edges);
  }

  pareto_dijkstra<my_label, lower_bounds> pd(
      sched_.node_count_, sched_.station_nodes_[DUMMY_TARGET_IDX].get(),
      start_labels, additional_edges, lb, label_store_);

  MOTIS_START_TIMING(pareto_dijkstra_timing);
  auto& results = pd.search();
  MOTIS_STOP_TIMING(pareto_dijkstra_timing);

  auto stats = pd.get_statistics();
  stats.pareto_dijkstra_ = MOTIS_TIMING_MS(pareto_dijkstra_timing);

  std::vector<journey> journeys;
  journeys.resize(results.size());
  std::transform(begin(results), end(results), begin(journeys),
                 [this](my_label* label) {
                   return output::labels_to_journey(label, sched_);
                 });

  return search_result(stats, journeys);
}

void search::generate_ontrip_start_labels(
    station_node const* start_station_node, time const start_time,
    std::vector<my_label*>& start_labels, lower_bounds& lower_bounds) {
  start_labels.push_back(new (label_store_.create<my_label>()) my_label(
      start_station_node, nullptr, start_time, lower_bounds));
}

void search::generate_start_labels(time const from, time const to,
                                   station_node const* start_station_node,
                                   std::vector<my_label*>& indices,
                                   station_node const* real_start, int time_off,
                                   lower_bounds& lower_bounds) {
  for (auto const& edge : start_station_node->edges_) {
    generate_start_labels(from, to, start_station_node, edge.get_destination(),
                          indices, real_start, time_off, lower_bounds);
  }
}

void search::generate_start_labels(time const from, time const to,
                                   station_node const* start_station_node,
                                   node const* route_node,
                                   std::vector<my_label*>& start_labels,
                                   station_node const* real_start, int time_off,
                                   lower_bounds& lower_bounds) {
  for (auto const& edge : route_node->edges_) {
    if (edge.empty()) {
      continue;
    }

    time t = from + time_off;
    while (t <= to + time_off) {
      auto con = edge.get_connection(t);
      if (con == nullptr) {
        break;
      }

      t = con->d_time_;

      if (t > to + time_off) {
        break;
      }

      my_label* earlier = nullptr;
      if (real_start != nullptr) {
        earlier = new (label_store_.create<my_label>())
            my_label(real_start, nullptr, t - time_off, lower_bounds);
      }

      auto station_node_label = new (label_store_.create<my_label>())
          my_label(start_station_node, earlier, t, lower_bounds);

      auto route_node_label = new (label_store_.create<my_label>())
          my_label(route_node, station_node_label, t, lower_bounds);

      // TODO(Felix Guendling):
      // if (route_node_label->_travel_time[1] !=
      //     std::numeric_limits<uint16_t>::max()) {
      start_labels.push_back(route_node_label);
      // }

      t = t + 1;
    }
  }
}

}  // namespace routing
}  // namespace motis
