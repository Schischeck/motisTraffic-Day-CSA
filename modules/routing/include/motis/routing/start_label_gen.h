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
    if ((start_edge->to_ == start && Dir == search_dir::FWD) ||
        (start_edge->from_ == start && Dir == search_dir::BWD)) {
      for (auto const& e : query_edges) {
        if (e.from_ != start) {
          continue;
        } else if (!e.to_->is_station_node() ||
                   (e.type() != edge::TIME_DEPENDENT_MUMO_EDGE &&
                    e.type() != edge::MUMO_EDGE)) {
          throw std::runtime_error("unsupported edge type");
        }

        auto const d = Dir == search_dir::FWD ? e.m_.foot_edge_.time_cost_
                                              : -e.m_.foot_edge_.time_cost_;
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
      generate_start_labels(
          mem, lbs, start_edge, nullptr,
          (Dir == search_dir::FWD ? start_edge->to_ : start_edge->from_)
              ->get_station(),
          0, interval_begin, interval_end, interval_end, labels);
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
    std::vector<edge const*> edges;
    if (Dir == search_dir::FWD) {
      for (auto const& e : station->edges_) {
        if (e.type() != edge::INVALID_EDGE) {
          edges.push_back(&e);
        }
      }
    } else {
      for (auto const& e : station->incoming_edges_) {
        if (e->type() != edge::INVALID_EDGE) {
          edges.push_back(e);
        }
      }
    }

    for (auto const& e : edges) {
      auto rn = e->get_destination<Dir>();
      if (!rn->is_route_node()) {
        continue;
      }

      if (Dir == search_dir::FWD) {
        for (auto const& re : rn->edges_) {
          generate_start_labels(*e, re, mem, lbs, start_edge, query_edge, d,
                                departure_begin, departure_end,
                                edge_interval_end, labels);
        }
      } else {
        for (auto const& re : rn->incoming_edges_) {
          generate_start_labels(*e, *re, mem, lbs, start_edge, query_edge, d,
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

    auto const end_reached = [&departure_begin,
                              &departure_end](time const t) -> bool {
      if (Dir == search_dir::FWD) {
        return t > departure_end;
      } else {
        return t < departure_begin;
      }
    };

    auto const get_time = [](light_connection const* lcon) {
      return (Dir == search_dir::FWD) ? lcon->d_time_ : lcon->a_time_;
    };

    auto i = 0;
    auto t = (Dir == search_dir::FWD) ? departure_begin : departure_end;
    auto const max_start_labels = departure_end - departure_begin;
    while (!end_reached(t)) {
      auto con = re.get_connection<Dir>(t);

      if (con == nullptr || end_reached(get_time(con))) {
        break;
      }

      t = get_time(con);

      auto time_off =
          d + std::max(static_cast<int>(t) - d - edge_interval_end, 0);

      if (query_edge == nullptr) {
        auto l = mem.create<Label>(start_edge, nullptr, t, lbs);
        labels.push_back(mem.create<Label>(&e, l, t, lbs));
      } else {
        auto dep = (Dir == search_dir::FWD) ? t - time_off : t + time_off;
        auto l0 = mem.create<Label>(start_edge, nullptr, dep, lbs);
        auto l1 = mem.create<Label>(query_edge, l0, t, lbs);
        labels.push_back(mem.create<Label>(&e, l1, t, lbs));
      }

      (Dir == search_dir::FWD) ? ++t : --t;

      if (++i > max_start_labels) {
        throw std::system_error(error::too_many_start_labels);
      }
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
