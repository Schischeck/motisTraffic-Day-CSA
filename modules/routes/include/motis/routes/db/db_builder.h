#pragma once

#include <mutex>

#include "motis/module/message.h"
#include "motis/protocol/RoutesSeqResponse_generated.h"
#include "motis/routes/db/rocksdb.h"
#include "motis/routes/fbs/RouteIndex_generated.h"

namespace motis {
namespace routes {

using routing_sequence = typed_flatbuffer<motis::routes::RoutesSeqResponse>;
using routing_lookup = typed_flatbuffer<motis::routes::RouteLookup>;

struct database;

struct db_builder {
  db_builder(rocksdb_database& db) : db_(db){};

  rocksdb_database& db_;
  std::mutex m_;
};

}  // namespace routes
}  // namespace motis
