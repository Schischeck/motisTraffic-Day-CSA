#pragma once

#include <mutex>

#include "geo/polygon.h"

#include "motis/module/message.h"
#include "motis/routes/db/rocksdb.h"

#include "motis/protocol/RoutesSeqResponse_generated.h"
#include "motis/routes/fbs/RouteIndex_generated.h"

namespace motis {
namespace routes {

using routing_sequence = typed_flatbuffer<motis::routes::RoutesSeqResponse>;
using routing_lookup = typed_flatbuffer<motis::routes::RouteLookup>;
using key_pair =
    std::tuple<std::vector<std::string>, std::vector<uint32_t>, int>;

struct db_builder {
  db_builder(kv_database& db) : db_(db){};

  int append(std::vector<std::string> const& station_ids,
             std::vector<uint32_t> const& classes,
             std::vector<std::vector<geo::latlng>> const& lines);

  void finish();

  kv_database& db_;
  std::vector<key_pair> indices_;
  std::mutex m_;
  int index_ = 0;
};

}  // namespace routes
}  // namespace motis
