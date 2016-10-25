#include "motis/routes/db/db_initializer.h"

#include <string>

#include "motis/routes/db/database.h"

namespace motis {
namespace routes {

void db_initializer::append(
    std::vector<uint32_t> classes,
    std::vector<flatbuffers::Offset<flatbuffers::String>> station_ids,
    routing_sequence const& res) {
  std::lock_guard<std::mutex> lock(m_);
  db_.put(std::to_string(index_), res.to_string());
  flatbuffers::FlatBufferBuilder b;
  indices_.push_back(CreateRouteIndex(b, b.CreateVector(station_ids),
                                      b.CreateVector(classes), index_));
  index_++;
}

void db_initializer::finish() {
  flatbuffers::FlatBufferBuilder b;
  b.Finish(CreateRouteLookup(b, b.CreateVector(indices_)));
  routing_lookup l(std::move(b));
  db_.put("__index", l.to_string());
}

}  // namespace routes
}  // namespace motis
