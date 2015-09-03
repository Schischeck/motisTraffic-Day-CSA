#include "motis/railviz/railviz.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"

#include "motis/protocol/RailVizInit_generated.h"
#include "motis/protocol/RailVizStationDetail_generated.h"
#include "motis/protocol/RailVizStationDetailRequest_generated.h"
#include "motis/protocol/RailVizAllTrainsRequest_generated.h"
#include "motis/protocol/RailVizAllTrainsResponse_generated.h"

#include "motis/railviz/train_retriever.h"
#include "motis/railviz/error.h"

using namespace flatbuffers;
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

railviz::railviz()
    : ops_{{MsgContent_RailVizStationDetailRequest,
            std::bind(&railviz::station_info, this, p::_1, p::_2, p::_3)},
           {MsgContent_RailVizAllTrainsRequest,
            std::bind(&railviz::all_trains, this, p::_1, p::_2, p::_3)}} {}

railviz::~railviz() {}

void railviz::station_info(msg_ptr msg, webclient&, callback cb) {
  auto lock = synced_sched<schedule_access::RO>();

  auto req = msg->content<RailVizStationDetailRequest const*>();
  int index = req->station_index();
  auto const& stations = lock.sched().stations;
  if (index < 0 || index >= stations.size()) {
    return cb({}, error::station_index_out_of_bounds);
  }

  flatbuffers::FlatBufferBuilder b;
  b.Finish(
      CreateMessage(b, MsgContent_RailVizStationDetail,
                    CreateRailVizStationDetail(
                        b, b.CreateString(stations[index]->name)).Union()));
  return cb(make_msg(b), boost::system::error_code());
}

void railviz::all_trains(msg_ptr msg, webclient& client, callback cb) {
  auto lock = synced_sched<schedule_access::RO>();
  auto req = msg->content<RailVizAllTrainsRequest const*>();

  client.bounds = {{req->p1()->lat(), req->p1()->lng()},
                   {req->p2()->lat(), req->p2()->lng()}};
  client.time = req->time();

  // request trains for the next 5 minutes
  auto trains = train_retriever_->trains(
      unix_to_motistime(lock.sched().schedule_begin_, client.time),
      unix_to_motistime(lock.sched().schedule_begin_, client.time + (60 * 5)),
      1000, client.bounds);

  std::vector<Train> trains_output;
  for (auto const& t : trains) {
    light_connection const* con;
    edge const* e;
    std::tie(con, e) = t;
    trains_output.emplace_back(
        motis_to_unixtime(lock.sched().schedule_begin_, con->d_time),
        motis_to_unixtime(lock.sched().schedule_begin_, con->a_time),
        e->_from->get_station()->_id, e->_to->get_station()->_id,
        e->_from->_route);
  }

  FlatBufferBuilder b;
  b.Finish(
      CreateMessage(b, MsgContent_RailVizAllTrainsResponse,
                    CreateRailVizAllTrainsResponse(
                        b, b.CreateVectorOfStructs(trains_output)).Union()));
  cb(make_msg(b), {});
}

void railviz::init() {
  auto lock = synced_sched<schedule_access::RO>();
  train_retriever_ =
      std::unique_ptr<train_retriever>(new train_retriever(lock.sched()));
}

void railviz::on_open(sid session) {
  clients_.insert(std::make_pair(session, session));

  auto lock = synced_sched<schedule_access::RO>();

  std::vector<StationCoordinate> stations;
  for (auto const& station : lock.sched().stations) {
    stations.emplace_back(station->width, station->length);
  }

  flatbuffers::FlatBufferBuilder b;
  b.Finish(
      CreateMessage(b, MsgContent_RailVizInit,
                    CreateRailVizInit(b, b.CreateVectorOfStructs(stations),
                                      lock.sched().schedule_begin_,
                                      lock.sched().schedule_end_).Union()));
  send(make_msg(b), session);
}

void railviz::on_close(sid session) { clients_.erase(session); }

void railviz::on_msg(msg_ptr msg, sid session, callback cb) {
  auto client_it = clients_.find(session);
  if (client_it == end(clients_)) {
    return cb({}, error::client_not_registered);
  }

  auto it = ops_.find(msg->msg_->content_type());
  return it->second(msg, client_it->second, cb);
}

}  // namespace railviz
}  // namespace motis
