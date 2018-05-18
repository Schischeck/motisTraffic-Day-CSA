#pragma once

#include <string>

#include "utl/to_vec.h"

#include "motis/core/common/hash_helper.h"
#include "motis/core/common/hash_map.h"
#include "motis/core/common/typed_flatbuffer.h"

#include "motis/path/error.h"

#include "motis/path/fbs/PathIndex_generated.h"

namespace motis {
namespace path {

struct lookup_index {
  using class_t = uint32_t;
  using lookup_table_t = typed_flatbuffer<motis::path::PathLookup>;

  struct key {
    key() = default;
    key(std::vector<std::string> station_ids, class_t clasz)
        : station_ids_(std::move(station_ids)), clasz_(clasz) {}

    struct hash {
      std::size_t operator()(key const& k) const {
        std::size_t seed = 0;
        for (auto const& id : k.station_ids_) {
          motis::hash_combine(seed, id);
        }
        motis::hash_combine(seed, k.clasz_);
        return seed;
      }
    };

    bool operator==(key const& o) const {
      return std::tie(station_ids_, clasz_) ==
             std::tie(o.station_ids_, o.clasz_);
    }

    std::vector<std::string> station_ids_;
    class_t clasz_ = 0;
  };

  lookup_index() {
    map_.set_empty_key({{}, std::numeric_limits<class_t>::max()});
  }

  explicit lookup_index(std::string const& s)
      : lookup_index(lookup_table_t{s.size(), s.data()}) {}

  explicit lookup_index(lookup_table_t const& lut) {
    map_.set_empty_key({{}, std::numeric_limits<class_t>::max()});

    for (auto const& entry : *lut.get()->indices()) {
      auto const seq = utl::to_vec(*entry->station_ids(),
                                   [](auto const& id) { return id->str(); });
      for (auto const& clasz : *entry->classes()) {
        map_.insert({{seq, clasz}, entry->index()});
      }
    }
  }

  std::string find(key const& k) const {
    auto const it = map_.find(k);
    if (it == end(map_)) {
      throw std::system_error(error::unknown_sequence);
    }
    return std::to_string(it->second);
  }

  hash_map<key, size_t, key::hash> map_;
};

}  // namespace path
}  // namespace motis
