#pragma once

#include <algorithm>
#include <limits>
#include <map>

#include "geo/latlng.h"
#include "geo/polyline.h"

#include "motis/routes/prepare/source_spec.h"
#include "motis/routes/prepare/station_sequences.h"

namespace motis {
namespace routes {

struct node_ref {
  static constexpr auto kInvalidRef = std::numeric_limits<size_t>::max();

  node_ref() = default;
  explicit node_ref(geo::latlng coords) : node_ref(coords, kInvalidRef) {}
  node_ref(geo::latlng coords, size_t id) : coords_(coords), id_(id) {}

  geo::latlng coords_;
  size_t router_id_;
  size_t id_;
};

struct routing_result {
  routing_result(source_spec s, double weight) : source_(s), weight_(weight) {}

  source_spec source_;
  double weight_;
};

struct routing_strategy {

  routing_strategy(std::size_t router_id) : router_id_(router_id) {}
  virtual ~routing_strategy() = default;

  virtual std::vector<node_ref> close_nodes(node_ref const& station) = 0;
  virtual std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to) = 0;
  virtual geo::polyline get_polyline(node_ref const& from,
                                     node_ref const& to) = 0;

  size_t router_id_;
};

}  // routes
}  // motis
