#pragma once

#include <mutex>

#include "geo/polygon.h"

#include "motis/module/message.h"
#include "motis/path/db/rocksdb.h"

#include "motis/path/fbs/PathIndex_generated.h"
#include "motis/protocol/PathSeqResponse_generated.h"

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
  explicit db_builder(std::unique_ptr<kv_database> db) : db_(std::move(db)) {}

  int append(std::vector<std::string> const& station_ids,
             std::vector<uint32_t> const& classes,
             std::vector<std::vector<geo::latlng>> const& lines,
             std::vector<sequence_info> const& sequence_infos);

  void finish();

  std::unique_ptr<kv_database> db_;
  std::vector<key_pair> indices_;
  std::mutex m_;
  int index_ = 0;
};

}  // namespace path
}  // namespace motis
