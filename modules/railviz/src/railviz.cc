#include "motis/railviz/railviz.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"

#include "motis/module/api.h"

#include "motis/protocol/RailVizStations_generated.h"
#include "motis/protocol/RailvizStationDetail_generated.h"
#include "motis/protocol/RailvizStationDetailRequest_generated.h"

#include "motis/railviz/train_retriever.h"

using namespace motis::module;
using namespace motis::logging;
namespace po = boost::program_options;

namespace motis {
namespace railviz {

po::options_description railviz::desc() {
  po::options_description desc("Railviz Module");
  return desc;
}

void railviz::print(std::ostream& out) const {}

msg_ptr station_info(railviz* r, msg_ptr const& msg) {
  auto& req = *reinterpret_cast<RailVizStationDetailRequest const*>(
                  msg->msg_->content());
  int index = req.station_index();
  auto const& stations = r->schedule_->stations;
  if (index < 0 || index >= stations.size()) {
    return {};
  }

  flatbuffers::FlatBufferBuilder b;
  b.Finish(CreateMessage(
      b, MsgContent_RailVizStationDetail,
      CreateRailVizStationDetail(
          b, b.CreateString(stations[index]->name.to_string())).Union()));
  return make_msg(b);
}

railviz::railviz()
    : ops_({{MsgContent_RailVizStationDetailRequest, station_info}}) {}

railviz::~railviz() {}

void railviz::init() {
  scoped_timer geo_index_timer("train retriever init");
  train_retriever_ =
      std::unique_ptr<train_retriever>(new train_retriever(*schedule_));
}

void railviz::on_open(sid session) {
  std::vector<Position> stations;
  for (auto const& station : schedule_->stations) {
    stations.emplace_back(station->width, station->length);
  }

  flatbuffers::FlatBufferBuilder b;
  b.Finish(CreateMessage(
      b, MsgContent_RailVizStations,
      CreateRailVizStations(b, b.CreateVectorOfStructs(stations)).Union()));
  (*send_)(make_msg(b), session);
}

void railviz::on_close(sid session) {}

msg_ptr railviz::on_msg(msg_ptr const& msg, sid session) {
  auto it = ops_.find(msg->msg_->content_type());
  if (it == end(ops_)) {
    return {};
  }
  return it->second(this, msg);
}

MOTIS_MODULE_DEF_MODULE(railviz)

}  // namespace railviz
}  // namespace motis
