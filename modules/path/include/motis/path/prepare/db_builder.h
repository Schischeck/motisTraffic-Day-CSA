#pragma once

#include <mutex>

#include "geo/latlng.h"
#include "geo/polygon.h"

#include "motis/core/common/hash_helper.h"
#include "motis/core/common/hash_map.h"
#include "motis/module/message.h"

#include "motis/path/db/lmdb.h"

#include "motis/path/fbs/PathIndex_generated.h"
#include "motis/protocol/PathSeqResponse_generated.h"

namespace std {

template <>
struct hash<std::pair<std::string, std::string>> {
  std::size_t operator()(std::pair<std::string, std::string> const& p) const {
    std::size_t seed = 0;
    motis::hash_combine(seed, p.first);
    motis::hash_combine(seed, p.second);
    return seed;
  }
};

}  // namespace std

namespace motis {
namespace path {

using routing_sequence = typed_flatbuffer<motis::path::PathSeqResponse>;
using routing_lookup = typed_flatbuffer<motis::path::PathLookup>;
using key_pair =
    std::tuple<std::vector<std::string>, std::vector<uint32_t>, int>;

struct sequence_info {
  sequence_info(size_t const idx, size_t const from, size_t const to,
                std::string type)
      : idx_(idx), from_(from), to_(to), type_(std::move(type)) {}

  size_t idx_;
  size_t from_;
  size_t to_;
  std::string type_;
};

struct db_builder {
  explicit db_builder(std::unique_ptr<kv_database> db) : db_(std::move(db)) {
    boxes_.set_empty_key({"", ""});
  }

  void append(std::vector<std::string> const& station_ids,
              std::vector<uint32_t> const& classes,
              std::vector<std::vector<geo::latlng>> const& lines,
              std::vector<sequence_info> const& sequence_infos);

  void finish();

private:
  void finish_index();
  void finish_boxes();

public:
  int index_ = 0;

  std::vector<key_pair> indices_;

  hash_map<std::pair<std::string, std::string>,
           std::pair<geo::latlng, geo::latlng>>
      boxes_;

  std::mutex m_;
  std::unique_ptr<kv_database> db_;
};

}  // namespace path
}  // namespace motis
