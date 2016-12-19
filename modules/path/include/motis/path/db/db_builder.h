#pragma once

#include <mutex>

#include "geo/polygon.h"

#include "motis/module/message.h"
#include "motis/path/db/rocksdb.h"

#include "motis/protocol/PathSeqResponse_generated.h"
#include "motis/path/fbs/PathIndex_generated.h"

namespace motis {
namespace path {

using routing_sequence = typed_flatbuffer<motis::path::PathSeqResponse>;
using routing_lookup = typed_flatbuffer<motis::path::PathLookup>;
using key_pair =
    std::tuple<std::vector<std::string>, std::vector<uint32_t>, int>;

struct sequence_info {
  int idx_;
  int from_;
  int to_;
  std::string type_;
};

struct db_builder {
  db_builder(kv_database& db) : db_(db){};

  int append(std::vector<std::string> const& station_ids,
             std::vector<uint32_t> const& classes,
             std::vector<std::vector<geo::latlng>> const& lines,
             std::vector<sequence_info> const& sequence_infos);

  void finish();

  kv_database& db_;
  std::vector<key_pair> indices_;
  std::mutex m_;
  int index_ = 0;
};

}  // namespace path
}  // namespace motis
