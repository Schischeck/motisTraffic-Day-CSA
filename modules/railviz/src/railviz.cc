#include "motis/railviz/railviz.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"

#include "motis/module/api.h"

#include "motis/protocol/RailVizInit_generated.h"
#include "motis/protocol/RailVizStationDetail_generated.h"
#include "motis/protocol/RailVizStationDetailRequest_generated.h"
#include "motis/protocol/RailVizAllTrainsRequest_generated.h"
#include "motis/protocol/RailVizAllTrainsResponse_generated.h"
#include "motis/protocol/RailVizRoutesOnTimeRequest_generated.h"
#include "motis/protocol/RailVizRoutesOnTimeResponse_generated.h"

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
            std::bind(&railviz::all_trains, this, p::_1, p::_2, p::_3)},
           {MsgContent_RailVizRoutesOnTimeRequest,
            std::bind(&railviz::routes_on_time, this, p::_1, p::_2, p::_3)}} {}

railviz::~railviz() {}

void railviz::station_info(msg_ptr msg, webclient&, callback cb) {
  auto lock = synced_sched<schedule_access::RO>();
  auto const& stations = lock.sched().stations;
  auto const& station_nodes = lock.sched().station_nodes;
  flatbuffers::FlatBufferBuilder b;

  auto req = msg->content<RailVizStationDetailRequest const*>();
  int index = req->station_index();

  if (index < 0 || index >= stations.size()) {
    return cb({}, error::station_index_out_of_bounds);
  }

  timetable timetable_ = timetable_retriever_.ordered_timetable_for_station(
      *station_nodes[index].get());

  std::vector<flatbuffers::Offset<TimetableEntry>> timetable_fb;
  for (const timetable_entry& entry : timetable_) {
    const light_connection* lc = std::get<0>(entry);
    const station_node* next_prev_station = std::get<1>(entry);
    const station_node* end_start_station = std::get<2>(entry);
    bool outgoing = std::get<3>(entry);
    unsigned int route = std::get<4>(entry);

    std::string line_name = lc->_full_con->con_info->line_identifier.to_string();
    std::string end_station_name = lock.sched().stations[end_start_station->_id].get()->name;


    std::time_t a_time = date_converter_.convert(lc->a_time);
    std::time_t d_time = date_converter_.convert(lc->d_time);
    int a_station, d_station;
    if (std::get<3>(entry)) {
      a_station = next_prev_station->_id;
      d_station = index;
    } else {
      d_station = next_prev_station->_id;
      a_station = index;
    }
    TrainS t(d_time, a_time, d_station, a_station, route);

    timetable_fb.push_back(CreateTimetableEntry(
        b, b.CreateString(line_name), &t, b.CreateString(end_station_name), end_start_station->_id, outgoing));
  }

  b.Finish(
      CreateMessage(b, MsgContent_RailVizStationDetail,
                    CreateRailVizStationDetail(
                        b, b.CreateString(stations[index]->name.to_string()),
                        index,
                        b.CreateVector(timetable_fb)).Union()));
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
      date_converter_.convert_to_motis(client.time),
      date_converter_.convert_to_motis(client.time + (60 * 5)), 1000,
      client.bounds);

  std::vector<TrainT> trains_output;
  for (auto const& t : trains) {
    light_connection const* con;
    edge const* e;
    std::tie(con, e) = t;
    trains_output.emplace_back(date_converter_.convert(con->d_time),
                               date_converter_.convert(con->a_time),
                               e->_from->get_station()->_id,
                               e->_to->get_station()->_id, e->_from->_route);
  }

  FlatBufferBuilder b;
  b.Finish(
      CreateMessage(b, MsgContent_RailVizAllTrainsResponse,
                    CreateRailVizAllTrainsResponse(
                        b, b.CreateVectorOfStructs(trains_output)).Union()));
  cb(make_msg(b), {});
}

void railviz::routes_on_time(msg_ptr msg, webclient &client, callback cb) {
    auto lock = synced_sched<schedule_access::RO>();
    auto const& stations = lock.sched().stations;
    auto req = msg->content<RailVizRoutesOnTimeRequest const*>();
    flatbuffers::FlatBufferBuilder b;

    // should we make this?
    client.time = req->time();

    std::vector<route> routes = timetable_retriever_.get_routes_on_time(req->route_id(), date_converter_.convert_to_motis(client.time));
    std::vector<flatbuffers::Offset<RailVizRoutesOnTimeEntry>> route_on_time_entries;

    for (auto r : routes) {
        for (auto re : r) {
            station_node const* stn;
            light_connection const* lcn;
            std::tie(stn, lcn) = re;
            StationS s(stn->_id, lcn->a_time, lcn->d_time);
            route_on_time_entries.push_back(CreateRailVizRoutesOnTimeEntry(
                                    b, b.CreateString(stations[stn->_id]->name.to_string()),
                                    &s));
        }
    }

    b.Finish(
        CreateMessage(b, MsgContent_RailVizRoutesOnTimeResponse,
                      CreateRailVizRoutesOnTimeResponse(
                         b, b.CreateVector(route_on_time_entries)).Union()));
    return cb(make_msg(b), boost::system::error_code());
}

void railviz::init() {
  auto lock = synced_sched<schedule_access::RO>();
  train_retriever_ =
      std::unique_ptr<train_retriever>(new train_retriever(lock.sched()));
  date_converter_.set_date_manager(lock.sched().date_mgr);
  timetable_retriever_.init(lock.sched());
}

void railviz::on_open(sid session) {
  clients_.emplace(session, session);

  auto lock = synced_sched<schedule_access::RO>();

  std::vector<StationCoordinate> stations;
  for (auto const& station : lock.sched().stations) {
    stations.emplace_back(station->width, station->length);
  }

  flatbuffers::FlatBufferBuilder b;
  b.Finish(CreateMessage(
      b, MsgContent_RailVizInit,
      CreateRailVizInit(
          b, b.CreateVectorOfStructs(stations),
          date_converter_.convert(lock.sched().date_mgr.first_date()),
          date_converter_.convert(lock.sched().date_mgr.last_date()) +
              MINUTES_A_DAY * 60).Union()));
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

MOTIS_MODULE_DEF_MODULE(railviz)

}  // namespace railviz
}  // namespace motis
