#pragma once

#include <algorithm>
#include <limits>
#include <map>

#include "geo/latlng.h"
#include "geo/polyline.h"

#include "motis/path/prepare/schedule/station_sequences.h"
#include "motis/path/prepare/source_spec.h"

namespace motis {
namespace path {

using strategy_id_t = size_t;
constexpr auto kInvalidStrategyId = std::numeric_limits<strategy_id_t>::max();

struct node_ref_id {
  node_ref_id() = default;
  node_ref_id(uint32_t id) : id_(id) {}
  node_ref_id(uint32_t relation_id, uint32_t id)
      : relation_id_(relation_id), id_(id) {}

  uint64_t relation_id_ : 32;
  uint64_t id_ : 32;
};

struct node_ref {
  node_ref() = default;
  node_ref(geo::latlng coords, node_ref_id id, strategy_id_t router_id)
      : coords_(coords), id_(id), router_id_(router_id) {}

  strategy_id_t strategy_id() const { return router_id_; };

  geo::latlng coords_;
  node_ref_id id_;
  strategy_id_t router_id_;  // XXX rename
};

struct routing_result {
  routing_result()
      : source_(),
        router_id_(kInvalidStrategyId),
        weight_(std::numeric_limits<double>::infinity()),
        valid_(false) {}

  routing_result(source_spec s, size_t router_id, double weight)
      : source_(s), router_id_(router_id), weight_(weight), valid_(true) {}

  strategy_id_t strategy_id() const { return router_id_; };

  source_spec source_;
  strategy_id_t router_id_;  // XXX rename
  double weight_;
  bool valid_;
};

struct routing_strategy {
  explicit routing_strategy(strategy_id_t const strategy_id)
      : strategy_id_(strategy_id) {}
  virtual ~routing_strategy() = default;

  virtual std::vector<node_ref> close_nodes(geo::latlng const&) = 0;
  virtual std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to) = 0;
  virtual geo::polyline get_polyline(node_ref const& from,
                                     node_ref const& to) const = 0;

  strategy_id_t strategy_id() const { return strategy_id_; }

private:
  strategy_id_t strategy_id_;
};

}  // namespace path
}  // motis
