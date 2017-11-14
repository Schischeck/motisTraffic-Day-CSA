#pragma once

#include "utl/to_vec.h"

#include "motis/core/common/hash_map.h"
#include "motis/core/common/timing.h"
#include "motis/core/schedule/schedule.h"
#include "motis/routing/lower_bounds.h"
#include "motis/routing/output/labels_to_journey.h"
#include "motis/routing/pareto_dijkstra.h"

namespace motis {
namespace routing {

struct search_query {
  search_query()
      : sched_(nullptr),
        mem_(nullptr),
        from_(nullptr),
        to_(nullptr),
        interval_begin_(0),
        interval_end_(0),
        extend_interval_earlier_(0),
        extend_interval_later_(0),
        min_journey_count_(0) {}

  schedule const* sched_;
  mem_manager* mem_;
  node const* from_;
  station_node const* to_;
  time interval_begin_;
  time interval_end_;
  bool extend_interval_earlier_;
  bool extend_interval_later_;
  std::vector<edge> query_edges_;
  unsigned min_journey_count_;
};

struct search_result {
  search_result() = default;
  search_result(statistics stats, std::vector<journey> journeys,
                time interval_begin, time interval_end)
      : stats_(std::move(stats)),  // NOLINT
        journeys_(std::move(journeys)),
        interval_begin_(interval_begin),
        interval_end_(interval_end) {}
  explicit search_result(unsigned travel_time_lb) : stats_(travel_time_lb) {}
  statistics stats_;
  std::vector<journey> journeys_;
  time interval_begin_;
  time interval_end_;
};

template <search_dir Dir, typename StartLabelGenerator, typename Label>
struct search {
  static search_result get_connections(search_query const& q) {
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

    pareto_dijkstra<Dir, Label, lower_bounds> pd(q.sched_->node_count_, q.to_,
                                                 std::move(additional_edges),
                                                 lbs, *q.mem_);

    auto const add_start_labels = [&](time interval_begin, time interval_end) {
      pd.add_start_labels(StartLabelGenerator::generate(
          *q.sched_, *q.mem_, lbs, &start_edge, q.query_edges_, interval_begin,
          interval_end));
    };

    time const schedule_begin = SCHEDULE_OFFSET_MINUTES;
    time const schedule_end =
        (q.sched_->schedule_end_ - q.sched_->schedule_begin_) / 60;

    auto const map_to_interval = [&schedule_begin, &schedule_end](time t) {
      return std::min(schedule_end, std::max(schedule_begin, t));
    };

    add_start_labels(q.interval_begin_, q.interval_end_);

    MOTIS_START_TIMING(pareto_dijkstra_timing);
    auto max_interval_reached = false;
    auto interval_begin = q.interval_begin_;
    auto interval_end = q.interval_end_;
    while (!max_interval_reached) {
      max_interval_reached =
          (!q.extend_interval_earlier_ || interval_begin == schedule_begin) &&
          (!q.extend_interval_later_ || interval_end == schedule_end);

      pd.search();

      if (pd.get_results().size() >= q.min_journey_count_) {
        break;
      }

      auto const new_interval_begin = q.extend_interval_earlier_
                                          ? map_to_interval(interval_begin - 60)
                                          : interval_begin;
      auto const new_interval_end = q.extend_interval_later_
                                        ? map_to_interval(interval_end + 60)
                                        : interval_end;

      if (interval_begin != schedule_begin) {
        add_start_labels(new_interval_begin,
                         map_to_interval(interval_begin - 1));
      }

      if (interval_end != schedule_end) {
        add_start_labels(map_to_interval(interval_end + 1), new_interval_end);
      }

      interval_begin = new_interval_begin;
      interval_end = new_interval_end;
    }
    MOTIS_STOP_TIMING(pareto_dijkstra_timing);

    auto stats = pd.get_statistics();
    stats.travel_time_lb_ = MOTIS_TIMING_MS(travel_time_lb_timing);
    stats.transfers_lb_ = MOTIS_TIMING_MS(transfers_lb_timing);
    stats.pareto_dijkstra_ = MOTIS_TIMING_MS(pareto_dijkstra_timing);

    return search_result(stats,
                         utl::to_vec(pd.get_results(),
                                     [&q](Label* label) {
                                       return output::labels_to_journey(
                                           *q.sched_, label, Dir);
                                     }),
                         interval_begin, interval_end);
  }
};

}  // namespace routing
}  // namespace motis
