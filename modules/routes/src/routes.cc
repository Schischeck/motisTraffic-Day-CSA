#include "motis/routes/routes.h"

#include <memory>

#include "motis/core/common/transform_to_vec.h"
#include "motis/core/access/trip_access.h"
#include "motis/core/access/trip_iterator.h"
#include "motis/core/access/trip_section.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_call.h"

#include "motis/routes/db/rocksdb.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::osrm;
using namespace motis::access;

namespace motis {
namespace routes {

routes::routes() : module("Routes", "routes") {
  string_param(database_path_, "routesdb", "db", "/path/to/routesdb");
}

routes::~routes() = default;

void routes::init(registry& r) {
  db_ = std::make_unique<rocksdb_database>(database_path_);
  auto buf = db_->get("__index");
  lookup_ = std::make_unique<lookup_table>(buf.size(), buf.c_str());

  r.register_op("/routes/index",
                [this](msg_ptr const& m) { return index_routes(m); });
  r.register_op("/routes/station_seq",
                [this](msg_ptr const& m) { return station_seq_routes(m); });
  r.register_op("/routes/id_train",
                [this](msg_ptr const& m) { return id_train_routes(m); });
}

msg_ptr routes::index_routes(msg_ptr const& msg) {
  auto req = motis_content(RoutesIndexRequest, msg);
  return get_response(std::to_string(req->index()));
}

msg_ptr routes::station_seq_routes(msg_ptr const& msg) {
  auto req = motis_content(RoutesStationSeqRequest, msg);
  return get_response(lookup_index(*lookup_, req));
}

msg_ptr routes::id_train_routes(msg_ptr const& msg) {
  auto const& req = motis_content(RoutesIdTrainRequest, msg);
  auto const& sched = get_schedule();
  auto const& t = req->trip_id();
  auto const& trp = get_trip(sched, t->station_id()->str(), t->train_nr(),
                             t->time(), t->target_station_id()->str(),
                             t->target_time(), t->line_id()->str());

  auto const seq = transform_to_vec(
      access::stops(trp),
      [&sched](auto const& stop) { return stop.get_station(sched).eva_nr_; });
  auto const clasz = trip_section{trp, 0}.fcon().clasz_;
  return get_response(motis::routes::lookup_index(*lookup_, seq, clasz));
}

msg_ptr routes::get_response(std::string index) {
  auto buf = db_->get(index);
  return std::make_shared<message>(buf.size(), buf.c_str());
}

}  // namespace routes
}  // namespace motis
