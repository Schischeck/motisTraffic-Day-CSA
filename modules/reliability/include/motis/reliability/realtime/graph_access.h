#pragma once

#include <string>

#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/event.h"
#include "motis/core/schedule/nodes.h"
#include "motis/core/schedule/schedule.h"

#include "motis/reliability/distributions/distributions_container.h"
#include "motis/reliability/error.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/realtime/time_util.h"

namespace motis {
namespace reliability {
namespace realtime {
namespace graph_access {

inline ev_key get_ev_key(schedule const& sched, edge const* re,
                         uint32_t const train_nr, uint32_t const family,
                         std::string const& line_identifier,
                         motis::event_type const ev_type,
                         time const sched_time) {
  auto const& lcons = re->m_.route_edge_.conns_;
  for (std::size_t lcon_idx = 0; lcon_idx < lcons.size(); ++lcon_idx) {
    auto const& lc = lcons[lcon_idx];
    auto const& con_info = *lc.full_con_->con_info_;
    auto const k = ev_key{re, lcon_idx, ev_type};
    if (con_info.family_ == family && con_info.train_nr_ == train_nr &&
        con_info.line_identifier_ == line_identifier &&
        get_schedule_time(sched, k) == sched_time) {
      return k;
    }
  }
  return {};
}

inline ev_key get_ev_key(schedule const& sched, uint32_t const train_nr,
                         uint32_t const family,
                         std::string const& line_identifier,
                         station_node const* station,
                         motis::event_type const ev_type,
                         time const sched_time) {
  for (auto const& e : station->edges_) {
    if (!e.to_->is_route_node()) {
      continue;
    }

    ev_key k;
    if (ev_type == event_type::DEP) {
      for (auto const& re : e.to_->edges_) {
        if (re.empty()) {
          continue;
        }

        auto const k = get_ev_key(sched, &re, train_nr, family, line_identifier,
                                  ev_type, sched_time);
        if (k.valid()) {
          return k;
        }
      }
    } else {
      for (auto const& re : e.to_->incoming_edges_) {
        if (re->empty()) {
          continue;
        }

        auto const k = get_ev_key(sched, re, train_nr, family, line_identifier,
                                  ev_type, sched_time);
        if (k.valid()) {
          return k;
        }
      }
    }
  }

  return ev_key();
}

auto get_node_and_light_connection(
    distributions_container::container::key const& key, schedule const& sched) {
  auto const family =
      graph_accessor::find_family(sched.categories_, key.category_);
  if (!family.first) {
    throw std::system_error(error::failure);
  }
  auto ev =
      get_ev_key(sched, key.train_id_, family.second, key.line_identifier_,
                 sched.station_nodes_.at(key.station_index_),
                 (key.type_ == time_util::departure ? motis::event_type::DEP
                                                    : motis::event_type::ARR),
                 key.scheduled_event_time_);

  return std::make_pair(new node(nullptr, 0), new light_connection());  // TODO
}
}  // namespace graph_access
}  // namespace realtime
}  // namespace reliability
}  // namespace motis
