#pragma once

#include <mutex>

#include "motis/core/common/typed_flatbuffer.h"
#include "motis/protocol/RoutesSeqResponse_generated.h"
#include "motis/routes/fbs/RouteIndex_generated.h"
#include "flatbuffers/flatbuffers.h"

namespace motis {
namespace routes {

using routing_sequence = typed_flatbuffer<motis::routes::RoutesSeqResponse>;
using routing_lookup = typed_flatbuffer<motis::routes::RouteLookup>;

struct database;

struct db_initializer {
  db_initializer(database& db) : db_(db){};

  void append(std::vector<uint32_t> classes,
              std::vector<flatbuffers::Offset<flatbuffers::String>> station_ids,
              routing_sequence const& res);

  void finish();

  database& db_;
  int index_ = 0;
  std::vector<flatbuffers::Offset<RouteIndex>> indices_;
  std::mutex m_;
};

}  // namespace routes
}  // namespace motis
