#include "motis/routes/routes.h"

#include <memory>

#include "motis/core/access/trip_access.h"
#include "motis/core/access/trip_iterator.h"
#include "motis/core/access/trip_section.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_call.h"

#include "motis/routes/db/rocksdb.h"

#include "motis/protocol/RoutesIndexRequest_generated.h"
#include "motis/protocol/RoutesSeqResponse_generated.h"
#include "motis/protocol/RoutesStationSeqRequest_generated.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::osrm;
using namespace motis::access;

namespace motis {
namespace routes {

using routes_response = typed_flatbuffer<motis::routes::RoutesSeqResponse>;

routes::routes() : module("Routes", "routes") {
  string_param(database_path_, "routesdb", "db", "/path/to/routesdb");
}

routes::~routes() = default;

void routes::init(registry& r) {
  db_ = std::make_unique<rocksdb_database>(database_path_);
  auto buf = db_->get("__index");
  lookup_table lt(buf.size(), buf.c_str());
  lookup_ = std::make_unique<index_lookup>(lt);

  r.register_op("/routes/index_routes",
                [this](msg_ptr const& m) { return index_routes(m); });
  r.register_op("/routes/station_seq_routes",
                [this](msg_ptr const& m) { return station_seq_routes(m); });
}

msg_ptr routes::index_routes(msg_ptr const& msg) {
  auto req = motis_content(RoutesIndexRequest, msg);
  auto buf = db_->get(std::to_string(req->index()));
  auto res = routes_response(buf.size(), buf.c_str());
  return std::make_shared<message>(buf.size(), buf.c_str());
}

msg_ptr routes::station_seq_routes(msg_ptr const& msg) {
  auto req = motis_content(RoutesStationSeqRequest, msg);
  auto buf = db_->get(lookup_->lookup(req));
  auto res = routes_response(buf.size(), buf.c_str());
  return std::make_shared<message>(buf.size(), buf.c_str());
}

}  // namespace routes
}  // namespace motis
