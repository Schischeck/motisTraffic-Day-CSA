#pragma once

#include <unordered_map>

#include "motis/core/common/timing.h"
#include "motis/core/schedule/schedule.h"
#include "motis/routing/lower_bounds.h"
#include "motis/routing/output/labels_to_journey.h"
#include "motis/routing/pareto_dijkstra.h"

namespace motis {
namespace routing {

struct search_query {
  schedule const* sched_;
  memory_manager* mem_;
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
  statistics stats_;
  std::vector<journey> journeys_;
};

template <search_dir Dir, typename StartLabelGenerator, typename Label>
struct search {
  static search_result get_connections(search_query const& q) {
    q.mem_->reset();

    std::unordered_map<int, std::vector<simple_edge>> lb_graph_edges;
    for (auto const& e : q.query_edges_) {
      lb_graph_edges[e.to_->get_station()->id_].emplace_back(
          e.from_->get_station()->id_, e.get_minimum_cost());
    }

    lower_bounds lbs(Dir == search_dir::FWD ? q.sched_->lower_bounds_fwd_
                                            : q.sched_->lower_bounds_bwd_,
                     q.to_->id_, lb_graph_edges);
    lbs.travel_time_.run();
    lbs.transfers_.run();

    if (lbs.travel_time_[q.from_->id_] ==
        std::numeric_limits<uint32_t>::max()) {
      return search_result();
    }

    std::unordered_map<node const*, std::vector<edge>> additional_edges;
    for (auto const& e : q.query_edges_) {
      additional_edges[e.from_].push_back(e);
    }

    // TODO(Felix Guendling) do not use const cast
    auto mutable_node = const_cast<node*>(q.from_);  // NOLINT
    auto const start_edge = make_foot_edge(nullptr, mutable_node);
    pareto_dijkstra<Label, lower_bounds> pd(
        q.sched_->node_count_, q.to_,
        StartLabelGenerator::generate(*q.sched_, *q.mem_, lbs, &start_edge,
                                      q.query_edges_, q.interval_begin_,
                                      q.interval_end_),
        additional_edges, lbs, *q.mem_);

    MOTIS_START_TIMING(pareto_dijkstra_timing);
    auto& results = pd.search();
    MOTIS_STOP_TIMING(pareto_dijkstra_timing);

    auto stats = pd.get_statistics();
    stats.pareto_dijkstra_ = MOTIS_TIMING_MS(pareto_dijkstra_timing);

    std::vector<journey> journeys;
    journeys.resize(results.size());
    std::transform(begin(results), end(results), begin(journeys),
                   [&q](Label* label) {
                     return output::labels_to_journey(*q.sched_, label, Dir);
                   });

    return search_result(stats, journeys);
  }
};

}  // namespace routing
}  // namespace motis
