#pragma once

#include <stdexcept>
#include <utility>
#include <vector>

#include "motis/core/schedule/schedule.h"
#include "motis/routing/lower_bounds.h"
#include "motis/routing/mem_manager.h"

namespace motis {
namespace routing {

template <search_dir Dir, typename Label>
struct pretrip_gen {
  static std::vector<Label*> generate(schedule const& sched, mem_manager& mem,
                                      lower_bounds& lbs, edge const* start_edge,
                                      std::vector<edge> const& query_edges,
                                      time interval_begin, time interval_end) {
    std::vector<Label*> labels;

    auto const start = sched.station_nodes_.at(0).get();
    if (start_edge->to_ == start) {
      for (auto const& e : query_edges) {
        if (e.from_ != start) {
          continue;
        } else if (!e.to_->is_station_node() ||
                   (e.type() != edge::TIME_DEPENDENT_MUMO_EDGE &&
                    e.type() != edge::MUMO_EDGE)) {
          throw std::runtime_error("unsupported edge type");
        }

        auto const d = e.m_.foot_edge_.time_cost_;
        auto const td = e.type() == edge::TIME_DEPENDENT_MUMO_EDGE;
        auto const edge_interval_begin =
            td ? std::max(e.m_.foot_edge_.interval_begin_, interval_begin)
               : interval_begin;
        auto const edge_interval_end =
            td ? std::min(e.m_.foot_edge_.interval_end_, interval_end)
               : interval_end;
        auto const departure_begin = edge_interval_begin + d;
        auto const departure_end = interval_end + d;

        generate_start_labels(mem, lbs, start_edge, &e,
                              e.to_->as_station_node(), d, departure_begin,
                              departure_end, edge_interval_end, labels);
      }
    } else {
      generate_start_labels(mem, lbs, start_edge, nullptr,
                            start_edge->to_->get_station(), 0, interval_begin,
                            interval_end, interval_end, labels);
    }

    return labels;
  }

  static void generate_start_labels(mem_manager& mem,
                                    lower_bounds& lbs,  //
                                    edge const* start_edge,
                                    edge const* query_edge,
                                    station_node const* station,  //
                                    duration d,  //
                                    time departure_begin, time departure_end,
                                    time edge_interval_end,
                                    std::vector<Label*>& labels) {
    for (auto const& e : station->edges_) {
      if (!e.to_->is_route_node()) {
        continue;
      }

      auto rn = e.to_;
      if (Dir == search_dir::FWD) {
        for (auto const& re : rn->edges_) {
          generate_start_labels(e, re, mem, lbs, start_edge, query_edge, d,
                                departure_begin, departure_end,
                                edge_interval_end, labels);
        }
      } else {
        for (auto const& re : rn->incoming_edges_) {
          generate_start_labels(e, *re, mem, lbs, start_edge, query_edge, d,
                                departure_begin, departure_end,
                                edge_interval_end, labels);
        }
      }
    }
  }

  static void generate_start_labels(edge const& e, edge const& re,
                                    mem_manager& mem,
                                    lower_bounds& lbs,  //
                                    edge const* start_edge,
                                    edge const* query_edge,
                                    duration d,  //
                                    time departure_begin, time departure_end,
                                    time edge_interval_end,
                                    std::vector<Label*>& labels) {
    if (re.empty()) {
      return;
    }

    auto t = departure_begin;
    while (t <= departure_end) {
      // TODO(Felix Guendling) get_connection_reverse for BWD
      auto con = re.get_connection<Dir>(t);

      // TODO(Felix Guendling) < departure_begin for BWD
      if (con == nullptr || con->d_time_ > departure_end) {
        break;
      }

      t = con->d_time_;

      // TODO(Felix Guendling) ?
      auto time_off =
          d + std::max(static_cast<int>(t) - d - edge_interval_end, 0);

      if (query_edge == nullptr) {
        auto l = mem.create<Label>(start_edge, nullptr, t, lbs);
        labels.push_back(mem.create<Label>(&e, l, t, lbs));
      } else {
        // TODO (Felix Guendling) ?
        auto l0 = mem.create<Label>(start_edge, nullptr, t - time_off, lbs);
        auto l1 = mem.create<Label>(query_edge, l0, t, lbs);
        labels.push_back(mem.create<Label>(&e, l1, t, lbs));
      }

      // TODO (Felix Guendling) --t for BWD
      ++t;
    }
  }
};

template <search_dir Dir, typename Label>
struct ontrip_gen {
  static std::vector<Label*> generate(schedule const&, mem_manager& mem,
                                      lower_bounds& lbs, edge const* start_edge,
                                      std::vector<edge> const&,
                                      time interval_begin,
                                      time /* interval_end */) {
    return {mem.create<Label>(start_edge, nullptr, interval_begin, lbs)};
  }
};

}  // namespace routing
}  // namespace motis
