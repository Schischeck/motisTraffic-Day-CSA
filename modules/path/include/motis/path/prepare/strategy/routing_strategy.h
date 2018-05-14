#pragma once

#include <algorithm>
#include <limits>
#include <map>

#include "geo/latlng.h"
#include "geo/polyline.h"

#include "parser/util.h"

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

  strategy_id_t strategy_id_ = 0;
  node_ref_id_t id_ = 0;
  geo::latlng coords_;
};

struct routing_result {
  routing_result()
      : strategy_id_(kInvalidStrategyId),
        source_(),
        weight_(std::numeric_limits<double>::infinity()) {}

  routing_result(size_t strategy_id, source_spec source, double weight)
      : strategy_id_(strategy_id), source_(source), weight_(weight) {}

  strategy_id_t strategy_id() const { return strategy_id_; }
  bool is_valid() const { return strategy_id_ != kInvalidStrategyId; }

  strategy_id_t strategy_id_;
  source_spec source_;
  double weight_;
};

struct routing_result_matrix {
  using raw_results_t = std::vector<std::vector<routing_result>>;

  routing_result_matrix() = default;

  explicit routing_result_matrix(raw_results_t const* ptr,
                                 bool is_transposed = false)
      : ptr_(ptr), is_transposed_(is_transposed) {}

  explicit routing_result_matrix(raw_results_t results,
                                 bool is_transposed = false)
      : mem_(std::make_unique<raw_results_t>(std::move(results))),
        ptr_(mem_.get()),
        is_transposed_(is_transposed) {}

  routing_result_matrix(routing_result_matrix&&) = default;  // NOLINT
  routing_result_matrix& operator=(routing_result_matrix&&) =  // NOLINT
      default;

  routing_result_matrix(routing_result_matrix const&) = delete;
  routing_result_matrix& operator=(routing_result_matrix const&) = delete;

  void verify_dimensions(size_t const from, size_t const to) const {
    if (!is_transposed_) {
      verify(from == ptr_->size(), "from size mismatch (nt)");
      for (auto const& vec : *ptr_) {
        verify(to == vec.size(), "to size mismatch (nt)");
      }
    } else {
      verify(to == ptr_->size(), "to size mismatch (t!)");
      for (auto const& vec : *ptr_) {
        verify(from == vec.size(), "from size mismatch (t!)");
      }
    }
  }

  bool is_valid() const { return ptr_ != nullptr; }

  routing_result const& get(size_t const from, size_t const to) const {
    return is_transposed_ ? ptr_->at(to).at(from) : ptr_->at(from).at(to);
  }

  std::unique_ptr<raw_results_t> mem_;
  raw_results_t const* ptr_ = nullptr;
  bool is_transposed_ = false;
};

struct routing_strategy {
  routing_strategy() = delete;
  explicit routing_strategy(strategy_id_t const strategy_id)
      : strategy_id_(strategy_id) {}
  routing_strategy(routing_strategy const&) = default;
  routing_strategy(routing_strategy&&) = default;
  routing_strategy& operator=(routing_strategy const&) = default;
  routing_strategy& operator=(routing_strategy&&) = default;
  virtual ~routing_strategy() = default;

  virtual std::vector<node_ref> close_nodes(
      std::string const& station_id) const = 0;

  virtual bool is_cacheable() const = 0;
  virtual bool can_route(node_ref const&) const = 0;

  virtual routing_result_matrix find_routes(
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
