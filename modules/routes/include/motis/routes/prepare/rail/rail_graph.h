#pragma once

#include <memory>
#include <vector>

#include "geo/latlng.h"
#include "geo/polyline.h"

namespace motis {
namespace routes {

struct rail_node;
struct rail_link;

struct rail_graph {
  std::vector<std::unique_ptr<rail_node>> nodes_;
};

struct rail_node {
  rail_node(size_t idx, int64_t id, geo::latlng pos)
      : idx_(idx), id_(id), pos_(pos) {}
  size_t idx_; // idx in railway_graph.nodes_

  int64_t id_; // from osm
  geo::latlng pos_;

  std::vector<rail_link> links_;
};

struct rail_link {
  rail_link(int64_t id, geo::polyline polyline, size_t dist,
            rail_node const* from, rail_node const* to)
      : id_(std::move(id)),
        polyline_(std::move(polyline)),
        dist_(dist),
        from_(from),
        to_(to) {}

  int64_t id_; // from osm

  geo::polyline polyline_;
  size_t dist_;

  rail_node const* from_;
  rail_node const* to_;
};

}  // namespace routes
}  // namespace motis
