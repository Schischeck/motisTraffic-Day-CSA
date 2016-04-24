#pragma once

#include <stdexcept>
#include <utility>
#include <vector>

#include "motis/core/schedule/schedule.h"
#include "motis/routing/lower_bounds.h"
#include "motis/routing/memory_manager.h"

namespace motis {
namespace routing {

template <typename Label>
struct pretrip_gen {
  static std::vector<Label*> generate(schedule const& sched,
                                      memory_manager& mem, lower_bounds& lbs,
                                      station_node const* from,
                                      std::vector<edge> const& query_edges,
                                      time interval_begin, time interval_end) {
    std::vector<Label*> labels;

    auto const start = sched.station_nodes_.at(0).get();
    if (from == start) {
      for (auto const& e : query_edges) {
        if (e.from_ != start) {
          continue;
        } else if (!e.to_->is_station_node() ||
                   (e.type() != edge::TIME_DEPENDENT_MUMO_EDGE &&
                    e.type() != edge::MUMO_EDGE)) {
          throw std::runtime_error("unsupported edge type");
        }

        auto const d = e.get_edge_cost(interval_begin, nullptr).time_;
        auto const td = e.type() == edge::TIME_DEPENDENT_MUMO_EDGE;
        auto const edge_interval_begin =
            td ? e.m_.foot_edge_.interval_begin_ : interval_begin;
        auto const edge_interval_end =
            td ? e.m_.foot_edge_.interval_end_ : interval_end;
        auto const departure_begin = edge_interval_begin + d;
        auto const departure_end = interval_end + d;

        generate_start_labels(mem, lbs, start, e.to_->as_station_node(), d,
                              departure_begin, departure_end, interval_end,
                              edge_interval_end, labels);
      }
    } else {
      generate_start_labels(mem, lbs, nullptr, from, 0, interval_begin,
                            interval_end, interval_end, interval_end, labels);
    }

    return labels;
  }

  static void generate_start_labels(memory_manager& mem,
                                    lower_bounds& lbs,  //
                                    station_node const* real_start,
                                    station_node const* station,  //
                                    duration d,  //
                                    time departure_begin, time departure_end,
                                    time interval_end, time edge_interval_end,
                                    std::vector<Label*>& labels) {
    for (auto const& rn : station->get_route_nodes()) {
      for (auto const& re : rn->edges_) {
        if (re.empty()) {
          continue;
        }

        auto t = departure_begin;
        while (t <= interval_end) {
          auto con = re.get_connection(t);
          if (con == nullptr || con->d_time_ > departure_end) {
            break;
          }

          t = con->d_time_;

          auto time_off =
              d + std::max(static_cast<int>(t) - d - edge_interval_end, 0);
          auto l0 =
              real_start == nullptr
                  ? nullptr
                  : mem.create<Label>(real_start, nullptr, t - time_off, lbs);
          auto l1 = mem.create<Label>(station, l0, t, lbs);
          labels.push_back(mem.create<Label>(rn, l1, t, lbs));

          ++t;
        }
      }
    }
  }
};

template <typename Label>
struct ontrip_gen {
  static std::vector<Label*> generate(schedule const&, memory_manager& mem,
                                      lower_bounds& lbs,
                                      station_node const* from,
                                      std::vector<edge> const&,
                                      time interval_begin,
                                      time /* interval_end */) {
    return {mem.create<Label>(from, nullptr, interval_begin, lbs)};
  }
};

}  // namespace routing
}  // namespace motis
