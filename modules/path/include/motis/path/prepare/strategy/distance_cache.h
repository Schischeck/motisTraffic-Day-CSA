#pragma once

#include <atomic>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

#include "motis/core/common/hash_helper.h"
#include "motis/core/common/hash_map.h"

namespace motis {
namespace path {

struct distance_cache {
  struct key {
    key() = default;

    key(std::string from, std::string to, strategy_id_t strategy_id)
        : from_(std::move(from)),
          to_(std::move(to)),
          strategy_id_(strategy_id) {}

    struct hash {
      std::size_t operator()(motis::path::distance_cache::key const& k) const {
        std::size_t seed = 0;

        if (k.from_ < k.to_) {
          motis::hash_combine(seed, k.from_);
          motis::hash_combine(seed, k.to_);
        } else {
          motis::hash_combine(seed, k.to_);
          motis::hash_combine(seed, k.from_);
        }

        motis::hash_combine(seed, k.strategy_id_);
        return seed;
      }
    };

    bool operator==(key const& o) const {
      return strategy_id_ == o.strategy_id_ &&
             (std::tie(from_, to_) == std::tie(o.from_, o.to_) ||
              std::tie(to_, from_) == std::tie(o.from_, o.to_));
    }

    std::string from_, to_;
    strategy_id_t strategy_id_ = 0;
  };

  using value = std::vector<std::vector<routing_result>>;

  distance_cache() {
    map_.set_empty_key({"", "", std::numeric_limits<strategy_id_t>::max()});
  }

  routing_result_matrix get(key const& key) {
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    auto const it = map_.find(key);
    if (it == end(map_)) {
      ++miss_;
      return {};
    }

    ++hit_;
    return routing_result_matrix{it->second, it->first.from_ != key.from_};
  }

  void put(key const& key, routing_result_matrix const& matrix) {
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);

    if (map_.find(key) != end(map_)) {
      ++race_;
      return;  // insert race!
    }

    ++put_;
    if (matrix.ptr_ == nullptr) {
      mem_.emplace_back(nullptr);
    } else {
      mem_.push_back(std::make_unique<value>(*matrix.ptr_));
    }

    if (matrix.is_transposed_) {
      struct key t_key {
        key.to_, key.from_, key.strategy_id_
      };
      map_.insert({t_key, mem_.back().get()});

    } else {
      map_.insert({key, mem_.back().get()});
    }
  }

  void dump_stats() const {
    std::cout << " === distance cache ===\n"
              << " get: " << hit_ + miss_ << " (" << hit_ << " hit, " << miss_
              << " miss)\n"
              << " put: " << put_ << " (" << race_ << " race)\n";
  }

  std::shared_timed_mutex mutex_;

  std::vector<std::unique_ptr<value>> mem_;
  hash_map<key, value const*, key::hash> map_;

  std::atomic<size_t> miss_{0};
  std::atomic<size_t> hit_{0};
  std::atomic<size_t> put_{0};
  std::atomic<size_t> race_{0};
};

}  // namespace path
}  // namespace motis
