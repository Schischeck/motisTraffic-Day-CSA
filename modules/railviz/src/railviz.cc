#include "motis/railviz/railviz.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"

#include "motis/module/api.h"

#include "motis/protocol/RailVizStations_generated.h"
#include "motis/protocol/RailVizStationDetail_generated.h"
#include "motis/protocol/RailVizStationDetailRequest_generated.h"

#include "motis/railviz/train_retriever.h"
#include "motis/railviz/error.h"

using namespace motis::module;
using namespace motis::logging;
namespace po = boost::program_options;
namespace p = std::placeholders;

namespace motis {
namespace railviz {

po::options_description railviz::desc() {
  po::options_description desc("Railviz Module");
  return desc;
}

void railviz::print(std::ostream& out) const {}

void railviz::station_info(msg_ptr msg, callback cb) {
  auto lock = synced_sched<schedule_access::RO>();

  auto req = msg->content<RailVizStationDetailRequest const*>();
  int index = req->station_index();
  auto const& stations = lock.sched().stations;
  if (index < 0 || index >= stations.size()) {
    return cb({}, error::station_index_out_of_bounds);
  }

  flatbuffers::FlatBufferBuilder b;
  b.Finish(CreateMessage(
      b, MsgContent_RailVizStationDetail,
      CreateRailVizStationDetail(
          b, b.CreateString(stations[index]->name.to_string())).Union()));
  return cb(make_msg(b), boost::system::error_code());
}

railviz::railviz()
    : ops_{{MsgContent_RailVizStationDetailRequest,
            std::bind(&railviz::station_info, this, p::_1, p::_2)}} {}

railviz::~railviz() {}

void railviz::init() {
  scoped_timer geo_index_timer("train retriever init");

  auto lock = synced_sched<schedule_access::RO>();
  train_retriever_ =
      std::unique_ptr<train_retriever>(new train_retriever(lock.sched()));
}

void railviz::on_open(sid session) {
  auto sync = synced_sched<schedule_access::RO>();

  std::vector<Position> stations;
  for (auto const& station : sync.sched().stations) {
    stations.emplace_back(station->width, station->length);
  }

  flatbuffers::FlatBufferBuilder b;
  b.Finish(CreateMessage(
      b, MsgContent_RailVizStations,
      CreateRailVizStations(b, b.CreateVectorOfStructs(stations)).Union()));
  send(make_msg(b), session);
}

void railviz::on_close(sid session) {}

void railviz::on_msg(msg_ptr msg, sid, callback cb) {
  auto it = ops_.find(msg->msg_->content_type());
  return it->second(msg, cb);
}

MOTIS_MODULE_DEF_MODULE(railviz)

}  // namespace railviz
}  // namespace motis
