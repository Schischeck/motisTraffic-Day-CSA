#pragma once

#include <limits>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

#include "motis/common/core/hash_map.h"

namespace motis {
namespace path {

struct result_cache_key {
  struct hash {
    std::size_t operator()(result_cache_key const& k) const {
      std::size_t seed = 0;

      if (from_ < to_) {
        hash_combine(seed, k.from_);
        hash_combine(seed, k.to_);
      } else {
        hash_combine(seed, k.to_);
        hash_combine(seed, k.from_);
      }

      hash_combine(seed, k.strategy_id_);
      return seed;
    }

    bool operator==(result_cache_key const& o) const {
      return strategy_id_ == o.strategy_id_ &&
             (std::tie(from_, to_) == std::tie(o.from_, o.to_) ||
              std::tie(to_, from_) == std::tie(o.from_, o.to_));
    }

    std::string from_, to_;
    strategy_id_t strategy_id_;
  };

  using result_cache_value = std::vector<std::vector<routing_result>>;

  struct distance_result_cache {

    distance_result_cache() {
      map_.set_empty_key({"", "", std::numeric_limits<strategy_id_t>::max()});
    }

    result_cache_value const* find(result_cache_key const& key) {
      std::shared_lock<std::shared_timed_mutex> lock(mutex_);

      auto const it = map_.find(key);
      if (it == end(map_)) {
        return nullptr;
      }
      return *it;
    }

    void put(result_cache_key const& key, result_cache_value const& value) {
      std::unique_lock<std::shared_timed_mutex> lock(mutex_);

      mem_.push_back(std::make_unique<result_cache_value>(value));
      map_[key] = value;
    }

    std::shared_timed_mutex mutex_;

    std::vector<std::unique_ptr<result_cache_value>> mem_;
    hash_map<result_cache_key, result_cache_value*> map_;
  };

}  // namespace path
}  // namespace motis
