#include "motis/rt/bfs.h"

#include <queue>

namespace motis {
namespace rt {

std::set<edge const*> route_bfs(ev_key const& k, bfs_direction const dir) {
  std::set<edge const*> visited;
  std::queue<edge const*> q;
  visited.insert(k.route_edge_);
  q.push(k.route_edge_);
  while (!q.empty()) {
    auto const e = q.front();
    q.pop();

    if (dir == bfs_direction::BOTH || dir == bfs_direction::BACKWARD) {
      for (auto const& in : e->from_->incoming_edges_) {
        if (in->empty()) {
          continue;
        }

        auto res = visited.insert(in);
        if (res.second) {
          q.push(in);
        }
      }
    }

    if (dir == bfs_direction::BOTH || dir == bfs_direction::FORWARD) {
      for (auto const& out : e->to_->edges_) {
        if (out.empty()) {
          continue;
        }

        auto res = visited.insert(&out);
        if (res.second) {
          q.push(&out);
        }
      }
    }
  }
  return visited;
}

std::set<ev_key> trip_bfs(ev_key const& k, bfs_direction const dir) {
  std::set<ev_key> ev_keys;
  for (auto const& e : route_bfs(k, dir)) {
    auto const arr = ev_key{e, k.lcon_idx_, event_type::ARR};
    auto const dep = ev_key{e, k.lcon_idx_, event_type::DEP};

    auto const bad_arr = dir == bfs_direction::BACKWARD && k.is_departure() &&
                         arr == k.get_opposite();
    if (!bad_arr) {
      ev_keys.insert(arr);
    }

    auto const bad_dep = dir == bfs_direction::FORWARD && k.is_arrival() &&
                         dep == k.get_opposite();
    if (!bad_dep) {
      ev_keys.insert(dep);
    }
  }
  return ev_keys;
}

}  // namespace rt
}  // namespace motis
