#pragma once

#include "motis/core/common/hash_map.h"
#include "motis/core/common/timing.h"
#include "motis/core/common/transform_to_vec.h"
#include "motis/core/schedule/schedule.h"
#include "motis/routing/lower_bounds.h"
#include "motis/routing/output/labels_to_journey.h"
#include "motis/routing/pareto_dijkstra.h"

namespace motis {
namespace routing {

struct search_query {
  schedule const* sched_;
  mem_manager* mem_;
  node const* from_;
  station_node const* to_;
  time interval_begin_;
  time interval_end_;
  std::vector<edge> query_edges_;
};

struct search_result {
  search_result() = default;
  search_result(statistics stats, std::vector<journey> journeys)  // NOLINT
      : stats_(stats),
        journeys_(std::move(journeys)) {}
  explicit search_result(unsigned travel_time_lb) : stats_(travel_time_lb) {}
  statistics stats_;
  std::vector<journey> journeys_;
};

template <search_dir Dir, typename StartLabelGenerator, typename Label>
struct search {
  static search_result get_connections(search_query const& q) {
    q.mem_->reset();

    hash_map<int, std::vector<simple_edge>> travel_time_lb_graph_edges;
    hash_map<int, std::vector<simple_edge>> transfers_lb_graph_edges;
    travel_time_lb_graph_edges.set_empty_key(std::numeric_limits<int>::max());
    transfers_lb_graph_edges.set_empty_key(std::numeric_limits<int>::max());
    for (auto const& e : q.query_edges_) {
      auto const orig_from = e.from_->get_station()->id_;
      auto const orig_to = e.to_->get_station()->id_;
      auto const from = (Dir == search_dir::FWD) ? orig_from : orig_to;
      auto const to = (Dir == search_dir::FWD) ? orig_to : orig_from;
      auto const ec = e.get_minimum_cost();
      travel_time_lb_graph_edges[to].emplace_back(from, ec.time_);
      transfers_lb_graph_edges[to].emplace_back(from, ec.transfer_ ? 1 : 0);
    }

    lower_bounds lbs(
        *q.sched_,  //
        Dir == search_dir::FWD ? q.sched_->travel_time_lower_bounds_fwd_
                               : q.sched_->travel_time_lower_bounds_bwd_,
        Dir == search_dir::FWD ? q.sched_->transfers_lower_bounds_fwd_
                               : q.sched_->transfers_lower_bounds_bwd_,
        q.to_->id_, travel_time_lb_graph_edges, transfers_lb_graph_edges);

    MOTIS_START_TIMING(travel_time_lb_timing);
    lbs.travel_time_.run();
    MOTIS_STOP_TIMING(travel_time_lb_timing);

    if (!lbs.travel_time_.is_reachable(q.from_->id_)) {
      return search_result(MOTIS_TIMING_MS(travel_time_lb_timing));
    }

    MOTIS_START_TIMING(transfers_lb_timing);
    lbs.transfers_.run();
    MOTIS_STOP_TIMING(transfers_lb_timing);

    hash_map<node const*, std::vector<edge>> additional_edges;
    additional_edges.set_empty_key(nullptr);
    for (auto const& e : q.query_edges_) {
      additional_edges[e.get_source<Dir>()].push_back(e);
    }

    auto mutable_node = const_cast<node*>(q.from_);  // NOLINT
    auto const start_edge = Dir == search_dir::FWD
                                ? make_foot_edge(nullptr, mutable_node)
                                : make_foot_edge(mutable_node, nullptr);
    pareto_dijkstra<Dir, Label, lower_bounds> pd(
        q.sched_->node_count_, q.to_,
        StartLabelGenerator::generate(*q.sched_, *q.mem_, lbs, &start_edge,
                                      q.query_edges_, q.interval_begin_,
                                      q.interval_end_),
        std::move(additional_edges), lbs, *q.mem_);

    MOTIS_START_TIMING(pareto_dijkstra_timing);
    pd.search();
    MOTIS_STOP_TIMING(pareto_dijkstra_timing);

    auto stats = pd.get_statistics();
    stats.pareto_dijkstra_ = MOTIS_TIMING_MS(pareto_dijkstra_timing);
    stats.travel_time_lb_ = MOTIS_TIMING_MS(travel_time_lb_timing);
    stats.transfers_lb_ = MOTIS_TIMING_MS(transfers_lb_timing);
    stats.pareto_dijkstra_ = MOTIS_TIMING_MS(pareto_dijkstra_timing);

    return search_result(
        stats, transform_to_vec(pd.get_results(), [&q](Label* label) {
          return output::labels_to_journey(*q.sched_, label, Dir);
        }));
  }
};

}  // namespace routing
}  // namespace motis
