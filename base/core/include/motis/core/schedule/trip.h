#pragma once

#include <cinttypes>

#include <string>
#include <utility>
#include <vector>

#include "motis/core/common/hash_helper.h"
#include "motis/core/schedule/edges.h"
#include "motis/core/schedule/nodes.h"

namespace motis {

struct primary_trip_id {
  primary_trip_id() = default;
  primary_trip_id(uint32_t station_id, uint32_t train_nr, motis::time time)
      : station_id_(station_id), train_nr_(train_nr), time_(time) {}

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

  uint64_t station_id_ : 31;
  uint64_t train_nr_ : 17;
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
    return std::tie(*reinterpret_cast<uint64_t const*>(&lhs), lhs.line_id_) <
           std::tie(*reinterpret_cast<uint64_t const*>(&rhs), rhs.line_id_);
  }

  friend bool operator==(secondary_trip_id const& lhs,
                         secondary_trip_id const& rhs) {
    return std::tie(*reinterpret_cast<uint64_t const*>(&lhs), lhs.line_id_) ==
           std::tie(*reinterpret_cast<uint64_t const*>(&rhs), rhs.line_id_);
  }

  std::time_t get_target_time() const {
    return static_cast<std::time_t>(target_time_);
  }

  uint64_t target_station_id_ : 32;
  uint64_t target_time_ : 16;
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
    route_edge() = default;

    explicit route_edge(edge const* e) : route_node_(e->from_) {
      for (std::size_t i = 0; i < route_node_->edges_.size(); ++i) {
        if (&route_node_->edges_[i] == e) {
          outgoing_edge_idx_ = i;
          return;
        }
      }
      assert(false);
    }

    edge* get_edge() const {
      assert(outgoing_edge_idx_ < route_node_->edges_.size());
      return &route_node_->edges_[outgoing_edge_idx_];
    }

    node* route_node_;
    std::size_t outgoing_edge_idx_;
  };

  explicit trip(full_trip_id id)
      : id_(std::move(id)), edges_(nullptr), lcon_idx_(0) {}

  full_trip_id id_;
  std::vector<route_edge> const* edges_;
  size_t lcon_idx_;
};

}  // namespace motis

namespace std {

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
