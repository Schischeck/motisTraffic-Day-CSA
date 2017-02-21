#pragma once

#include <cinttypes>

#include <string>
#include <utility>
#include <vector>

#include "utl/to_vec.h"

#include "motis/core/common/hash_helper.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/nodes.h"

namespace motis {

struct primary_trip_id {
  primary_trip_id() = default;
  primary_trip_id(uint32_t station_id, uint32_t train_nr, motis::time time)
      : train_nr_(train_nr), station_id_(station_id), time_(time) {}

  friend bool operator<(primary_trip_id const& lhs,
                        primary_trip_id const& rhs) {
    return *reinterpret_cast<uint64_t const*>(&lhs) <
           *reinterpret_cast<uint64_t const*>(&rhs);
  }

  friend bool operator==(primary_trip_id const& lhs,
                         primary_trip_id const& rhs) {
    return *reinterpret_cast<uint64_t const*>(&lhs) ==
           *reinterpret_cast<uint64_t const*>(&rhs);
  }

  motis::time get_time() const { return static_cast<motis::time>(time_); }
  uint32_t get_train_nr() const { return static_cast<uint32_t>(train_nr_); }

  uint64_t train_nr_ : 17;
  uint64_t station_id_ : 31;
  uint64_t time_ : 16;
};

struct secondary_trip_id {
  secondary_trip_id() = default;
  secondary_trip_id(uint32_t target_station_id, uint16_t target_time,
                    std::string line_id)
      : target_station_id_(target_station_id),
        target_time_(target_time),
        line_id_(std::move(line_id)) {}

  friend bool operator<(secondary_trip_id const& lhs,
                        secondary_trip_id const& rhs) {
    return std::tie(lhs.target_station_id_, lhs.target_time_, lhs.line_id_) <
           std::tie(rhs.target_station_id_, rhs.target_time_, rhs.line_id_);
  }

  friend bool operator==(secondary_trip_id const& lhs,
                         secondary_trip_id const& rhs) {
    return std::tie(lhs.target_station_id_, lhs.target_time_, lhs.line_id_) ==
           std::tie(rhs.target_station_id_, rhs.target_time_, rhs.line_id_);
  }

  uint32_t target_station_id_;
  motis::time target_time_;
  std::string line_id_;
};

struct full_trip_id {
  friend bool operator<(full_trip_id const& lhs, full_trip_id const& rhs) {
    return std::tie(lhs.primary_, lhs.secondary_) <
           std::tie(rhs.primary_, rhs.secondary_);
  }

  friend bool operator==(full_trip_id const& lhs, full_trip_id const& rhs) {
    return std::tie(lhs.primary_, lhs.secondary_) ==
           std::tie(rhs.primary_, rhs.secondary_);
  }

  primary_trip_id primary_;
  secondary_trip_id secondary_;
};

struct trip {
  struct route_edge {
    route_edge() : route_node_(nullptr), outgoing_edge_idx_(0) {}

    route_edge(edge const* e) : route_edge() {  // NOLINT
      if (e != nullptr) {
        route_node_ = e->from_;
        for (std::size_t i = 0; i < route_node_->edges_.size(); ++i) {
          if (&route_node_->edges_[i] == e) {
            outgoing_edge_idx_ = i;
            return;
          }
        }
        assert(false);
      }
    }

    friend bool operator==(route_edge const& a, route_edge const& b) {
      return std::tie(a.route_node_, a.outgoing_edge_idx_) ==
             std::tie(b.route_node_, b.outgoing_edge_idx_);
    }

    friend bool operator<(route_edge const& a, route_edge const& b) {
      return std::tie(a.route_node_, a.outgoing_edge_idx_) <
             std::tie(b.route_node_, b.outgoing_edge_idx_);
    }

    bool valid() const { return route_node_ != nullptr; }

    edge* get_edge() const {
      assert(outgoing_edge_idx_ < route_node_->edges_.size());
      return &route_node_->edges_[outgoing_edge_idx_];
    }

    edge* operator->() const { return get_edge(); }

    operator edge*() const { return get_edge(); }

    node* route_node_;
    std::size_t outgoing_edge_idx_;
  };

  explicit trip(full_trip_id id)
      : id_(std::move(id)), edges_(nullptr), lcon_idx_(0) {}

  trip(full_trip_id id, std::vector<route_edge> const* edges, size_t lcon_idx)
      : id_(std::move(id)), edges_(edges), lcon_idx_(lcon_idx) {}

  full_trip_id id_;
  std::vector<route_edge> const* edges_;
  size_t lcon_idx_;
};

}  // namespace motis

namespace std {

template <>
struct hash<motis::trip::route_edge> {
  std::size_t operator()(motis::trip::route_edge const& e) const {
    std::size_t seed = 0;
    motis::hash_combine(seed, e.route_node_);
    motis::hash_combine(seed, e.outgoing_edge_idx_);
    return seed;
  }
};

template <>
struct hash<motis::primary_trip_id> {
  std::size_t operator()(motis::primary_trip_id const& e) const {
    return *reinterpret_cast<uint64_t const*>(&e);
  }
};

template <>
struct hash<motis::secondary_trip_id> {
  std::size_t operator()(motis::secondary_trip_id const& e) const {
    std::size_t seed = 0;
    motis::hash_combine(seed, e.target_station_id_);
    motis::hash_combine(seed, e.target_time_);
    motis::hash_combine(seed, e.line_id_);
    return seed;
  }
};

template <>
struct hash<motis::full_trip_id> {
  std::size_t operator()(motis::full_trip_id const& e) const {
    std::size_t seed = 0;
    motis::hash_combine(seed, e.primary_);
    motis::hash_combine(seed, e.secondary_);
    return seed;
  }
};

}  // namespace std
