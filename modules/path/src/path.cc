#include "motis/path/path.h"

#include <memory>

#include "utl/to_vec.h"

#include "motis/core/common/logging.h"
#include "motis/core/access/trip_access.h"
#include "motis/core/access/trip_iterator.h"
#include "motis/core/access/trip_section.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_call.h"

#include "motis/path/db/kv_database.h"
#include "motis/path/lookup_index.h"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::osrm;
using namespace motis::access;

namespace motis {
namespace path {

path::path() : module("Path", "path") {
  string_param(database_path_, "pathdb", "db", "/path/to/pathdb");
}

path::~path() = default;

void path::init(registry& r) {
  db_ = load_db(database_path_);

  if (auto buf = db_->try_get("__index")) {
    lookup_ = std::make_unique<lookup_index>(*buf);
  } else {
    std::cout << "new"<< std::endl;

    message_creator mc;
    mc.Finish(
        CreatePathLookup(mc, mc.CreateVector<Offset<PathIndex>>({})));  // XXX ?
    lookup_ = std::make_unique<lookup_index>(
        lookup_index::lookup_table{std::move(mc)});
  }

  // XXX
  // std::cout << "sequences: " << std::endl;

  // for (auto const& entry : *lookup_->lookup_table_.get()->indices()) {
  //   std::cout << entry->index() << ":\n";
  //   std::cout << " ";
  //   for (auto const& station_id : *entry->station_ids()) {
  //     std::cout << station_id->str() << ", ";
  //   }
  //   std::cout << "\n ";
  //   for (auto const& cls : *entry->classes()) {
  //     std::cout << cls << ", ";
  //   }
  //   std::cout << std::endl;
  // }

  r.register_op("/path/index",
                [this](msg_ptr const& m) { return index_path(m); });
  r.register_op("/path/station_seq",
                [this](msg_ptr const& m) { return station_seq_path(m); });
  r.register_op("/path/id_train",
                [this](msg_ptr const& m) { return id_train_path(m); });
}

msg_ptr path::index_path(msg_ptr const& msg) const {
  auto req = motis_content(PathIndexRequest, msg);
  return get_response(std::to_string(req->index()));
}

msg_ptr path::station_seq_path(msg_ptr const& msg) const {
  auto req = motis_content(PathStationSeqRequest, msg);
  return get_response(lookup_->find(req));
}

msg_ptr path::id_train_path(msg_ptr const& msg) const {
  auto const& req = motis_content(PathIdTrainRequest, msg);
  auto const& sched = get_schedule();
  auto const& t = req->trip_id();
  auto const& trp = get_trip(sched, t->station_id()->str(), t->train_nr(),
                             t->time(), t->target_station_id()->str(),
                             t->target_time(), t->line_id()->str());

  auto const seq = utl::to_vec(access::stops(trp), [&sched](auto const& stop) {
    return stop.get_station(sched).eva_nr_;
  });
  auto const clasz = trip_section{trp, 0}.fcon().clasz_;
  return get_response(lookup_->find(seq, clasz));
}

msg_ptr path::get_response(std::string const& index) const {
  auto buf = db_->get(index);
  return std::make_shared<message>(buf.size(), buf.c_str());
}

}  // namespace path
}  // namespace motis
