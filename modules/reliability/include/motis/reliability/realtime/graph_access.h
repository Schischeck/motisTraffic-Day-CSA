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
                         uint32_t const train_nr,
                         std::vector<uint32_t> const& family,
                         std::string const& line_identifier,
                         motis::event_type const ev_type,
                         time const sched_time) {
  auto const& lcons = re->m_.route_edge_.conns_;
  for (std::size_t lcon_idx = 0; lcon_idx < lcons.size(); ++lcon_idx) {
    auto const& lc = lcons[lcon_idx];
    auto const& con_info = *lc.full_con_->con_info_;
    auto const k = ev_key{re, lcon_idx, ev_type};
    bool const family_found = std::find(family.begin(), family.end(),
                                        con_info.family_) != family.end();
    if (family_found && con_info.train_nr_ == train_nr &&
        con_info.line_identifier_ == line_identifier &&
        get_schedule_time(sched, k) == sched_time) {
      return k;
    }
  }
  return {};
}

inline ev_key get_ev_key(schedule const& sched, uint32_t const train_nr,
                         std::vector<uint32_t> const& family,
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

std::vector<uint32_t> find_family(
    std::vector<std::unique_ptr<category>> const& categories,
    std::string const& category_name) {
  auto to_lower = [](std::string str) -> std::string {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
  };
  std::vector<uint32_t> family;
  auto const c = to_lower(category_name);
  for (uint32_t f = 0; f < categories.size(); ++f) {
    if (to_lower(categories[f]->name_) == c) {
      family.push_back(f);
    }
  }
  return family;
}

auto get_node_and_light_connection(
    distributions_container::container::key const& key, schedule const& sched) {
  auto const family = find_family(sched.categories_, key.category_);
  if (family.empty()) {
    throw std::system_error(error::failure);
  }
  auto ev =
      get_ev_key(sched, key.train_id_, family, key.line_identifier_,
                 sched.station_nodes_.at(key.station_index_).get(),
                 (key.type_ == time_util::departure ? motis::event_type::DEP
                                                    : motis::event_type::ARR),
                 key.scheduled_event_time_);
  if (!ev.valid()) {
    throw std::system_error(error::failure);
  }
  auto const* route_node = key.type_ == time_util::departure
                               ? ev.route_edge_->from_
                               : ev.route_edge_->to_;
  auto const* lc = &ev.route_edge_->m_.route_edge_.conns_[ev.lcon_idx_];
  return std::make_pair(route_node, lc);
}
}  // namespace graph_access
}  // namespace realtime
}  // namespace reliability
}  // namespace motis
