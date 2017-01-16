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
using node_ref_id_t = size_t;

struct node_ref {
  node_ref() = default;
  node_ref(strategy_id_t const strategy_id, node_ref_id_t const id,
           geo::latlng const coords)
      : strategy_id_(strategy_id), id_(id), coords_(coords) {}

  strategy_id_t strategy_id() const { return strategy_id_; };

  friend bool operator==(node_ref const& a, node_ref const& b) {
    return std::tie(a.id_, a.strategy_id_, a.coords_) ==
           std::tie(b.id_, b.strategy_id_, b.coords_);
  }

  friend bool operator<(node_ref const& a, node_ref const& b) {
    return std::tie(a.id_, a.strategy_id_, a.coords_) <
           std::tie(b.id_, b.strategy_id_, b.coords_);
  }

  strategy_id_t strategy_id_;
  node_ref_id_t id_;

  geo::latlng coords_;
};

struct routing_result {
  routing_result()
      : strategy_id_(kInvalidStrategyId),
        source_(),
        weight_(std::numeric_limits<double>::infinity()) {}

  routing_result(size_t strategy_id, source_spec source, double weight)
      : strategy_id_(strategy_id),
        source_(std::move(source)),
        weight_(weight) {}

  strategy_id_t strategy_id() const { return strategy_id_; }
  bool is_valid() const { return strategy_id_ != kInvalidStrategyId; }

  strategy_id_t strategy_id_;
  source_spec source_;
  double weight_;
};

struct routing_strategy {
  explicit routing_strategy(strategy_id_t const strategy_id)
      : strategy_id_(strategy_id) {}
  virtual ~routing_strategy() = default;

  virtual std::vector<node_ref> close_nodes(
      std::string const& station_id) const = 0;

  virtual bool can_route(node_ref const&) const = 0;
  virtual std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from,
      std::vector<node_ref> const& to) const = 0;

  virtual geo::polyline get_polyline(node_ref const& from,
                                     node_ref const& to) const = 0;

  strategy_id_t strategy_id() const { return strategy_id_; }

private:
  strategy_id_t strategy_id_;
};

}  // namespace path
}  // namespace motis
