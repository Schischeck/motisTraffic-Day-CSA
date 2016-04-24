#pragma once

#include <unordered_map>

#include "motis/core/common/timing.h"
#include "motis/core/schedule/schedule.h"
#include "motis/routing/lower_bounds.h"
#include "motis/routing/output/labels_to_journey.h"
#include "motis/routing/pareto_dijkstra.h"

namespace motis {
namespace routing {

struct search_result {
  search_result() = default;
  search_result(statistics stats, std::vector<journey> journeys)  // NOLINT
      : stats_(stats),
        journeys_(std::move(journeys)) {}
  statistics stats_;
  std::vector<journey> journeys_;
};

template <typename StartLabelGenerator, typename Label>
struct search {
  static search_result get_connections(schedule const& sched,
                                       memory_manager& mem,
                                       station_node const* from,
                                       station_node const* to,
                                       std::vector<edge> const& query_edges,
                                       time interval_begin, time interval_end) {
    mem.reset();

    std::unordered_map<int, std::vector<simple_edge>> lb_graph_edges;
    for (auto const& e : query_edges) {
      lb_graph_edges[e.to_->get_station()->id_].emplace_back(
          e.from_->get_station()->id_, e.get_minimum_cost());
    }

    lower_bounds lbs(sched.lower_bounds_, to->id_, lb_graph_edges);
    lbs.travel_time_.run();
    lbs.transfers_.run();

    if (lbs.travel_time_[from->id_] == std::numeric_limits<uint32_t>::max()) {
      return search_result();
    }

    std::unordered_map<node const*, std::vector<edge>> additional_edges;
    for (auto const& e : query_edges) {
      additional_edges[e.from_].push_back(e);
    }

    pareto_dijkstra<my_label, lower_bounds> pd(
        sched.node_count_, to,
        StartLabelGenerator::generate(sched, mem, lbs, from, query_edges,
                                      interval_begin, interval_end),
        additional_edges, lbs, mem);

    MOTIS_START_TIMING(pareto_dijkstra_timing);
    auto& results = pd.search();
    MOTIS_STOP_TIMING(pareto_dijkstra_timing);

    auto stats = pd.get_statistics();
    stats.pareto_dijkstra_ = MOTIS_TIMING_MS(pareto_dijkstra_timing);

    std::vector<journey> journeys;
    journeys.resize(results.size());
    std::transform(begin(results), end(results), begin(journeys),
                   [&sched](my_label* label) {
                     return output::labels_to_journey(label, sched);
                   });

    return search_result(stats, journeys);
  }
};

}  // namespace routing
}  // namespace motis
