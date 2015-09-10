#include "motis/railviz/railviz.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"

#include "motis/module/api.h"

#include "motis/protocol/RailViz_init_generated.h"
#include "motis/protocol/RailViz_alltra_req_generated.h"
#include "motis/protocol/RailViz_alltra_res_generated.h"
#include "motis/protocol/RailViz_station_detail_req_generated.h"
#include "motis/protocol/RailViz_station_detail_res_generated.h"
#include "motis/protocol/RailViz_route_at_time_req_generated.h"
#include "motis/protocol/RailViz_route_at_time_res_generated.h"

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

std::vector<std::string> railviz::clasz_names = {"ICE", "IC", "N", "RE", "RB", "S", "U", "STR", "BUS", "X"};

railviz::railviz()
    : ops_{{MsgContent_RailViz_alltra_req,
            std::bind(&railviz::all_trains, this, p::_1, p::_2, p::_3)},
           {MsgContent_RailViz_station_detail_req,
            std::bind(&railviz::station_info, this, p::_1, p::_2, p::_3)},
           {MsgContent_RailViz_route_at_time_req,
            std::bind(&railviz::route_at_time, this, p::_1, p::_2, p::_3)}} {}

railviz::~railviz() {}

void railviz::all_trains(msg_ptr msg, webclient& client, callback cb) {
  auto lock = synced_sched<schedule_access::RO>();
  auto req = msg->content<RailViz_alltra_req const*>();
  FlatBufferBuilder b;

  client.bounds = {{req->p1()->lat(), req->p1()->lng()},
                   {req->p2()->lat(), req->p2()->lng()}};
  client.time = req->time();

  // request trains for the next 5 minutes
  auto trains = train_retriever_->trains(
      date_converter_.convert_to_motis(client.time),
      date_converter_.convert_to_motis(client.time + (60 * 5)), 1000,
      client.bounds);

  std::vector<RailVizTrain> trains_output;
  std::vector<flatbuffers::Offset<RailViz_alltra_res_route>> fb_routes;
  for (auto const& t : trains) {
    light_connection const* con;
    edge const* e;
    std::tie(con, e) = t;
    trains_output.emplace_back(date_converter_.convert(con->d_time),
                               date_converter_.convert(con->a_time),
                               e->_from->get_station()->_id,
                               e->_to->get_station()->_id, e->_from->_route);

    int route_id = e->_from->_route;
    if( req->with_routes() ) {
      std::vector<unsigned int> fb_route;
      std::vector<const motis::station_node*> stations = timetable_retriever_.stations_on_route(route_id);
      for( const motis::station_node* station_node_ : stations ) {
        fb_route.push_back(station_node_->_id);
      }
      fb_routes.push_back( CreateRailViz_alltra_res_route(b, b.CreateVector(fb_route) ));

    }
  }

  b.Finish(
      CreateMessage(b, MsgContent_RailViz_alltra_res,
                    CreateRailViz_alltra_res(
                        b, b.CreateVectorOfStructs(trains_output),
                        b.CreateVector(fb_routes)).Union()));
  cb(make_msg(b), {});
}

void railviz::station_info(msg_ptr msg, webclient&, callback cb) {
  auto lock = synced_sched<schedule_access::RO>();
  auto const& stations = lock.sched().stations;
  auto const& station_nodes = lock.sched().station_nodes;
  flatbuffers::FlatBufferBuilder b;

  auto req = msg->content<RailViz_station_detail_req const*>();
  int index = req->station_index();

  if (index < 0 || index >= stations.size()) {
    return cb({}, error::station_index_out_of_bounds);
  }

  timetable timetable_ = timetable_retriever_.ordered_timetable_for_station(
      *station_nodes[index].get());

  std::vector<flatbuffers::Offset<RailViz_station_detail_res_entry>> timetable_fb;
  for (const timetable_entry& entry : timetable_) {
    const light_connection* lc = std::get<0>(entry);
    const station_node* next_prev_station = std::get<1>(entry);
    const station_node* end_start_station = std::get<2>(entry);
    bool outgoing = std::get<3>(entry);
    unsigned int route = std::get<4>(entry);

    std::string line_name = lc->_full_con->con_info->line_identifier.to_string();
    std::string end_station_name = lock.sched().stations[end_start_station->_id].get()->name;


    //std::vector<std::string> clasz_names = {"ICE", "IC", "N", "RE", "RB", "S", "U", "STR", "BUS", "X"};

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
    RailVizTrain t(d_time, a_time, d_station, a_station, route);

    timetable_fb.push_back(CreateRailViz_station_detail_res_entry(
        b, b.CreateString(line_name), b.CreateString(railviz::clasz_names[lc->_full_con->clasz]), b.CreateString(lock.sched().category_names[lc->_full_con->con_info->family]), &t, b.CreateString(end_station_name), end_start_station->_id, outgoing));
  }

  b.Finish(
      CreateMessage(b, MsgContent_RailViz_station_detail_res,
                    CreateRailViz_station_detail_res(
                        b, b.CreateString(stations[index]->name.to_string()),
                        index,
                        b.CreateVector(timetable_fb)).Union()));
  return cb(make_msg(b), boost::system::error_code());
}

void railviz::route_at_time(msg_ptr msg, webclient &client, callback cb) {
  auto lock = synced_sched<schedule_access::RO>();
  auto const& stations = lock.sched().stations;

  RailViz_route_at_time_req const* req = msg->content<RailViz_route_at_time_req const*>();
  unsigned int station_id = req->station_id();
  motis::time departure_time = date_converter_.convert_to_motis(req->departure_time());
  unsigned int route_id = req->route_id();

  std::vector<route> routes = timetable_retriever_.get_routes_on_time(route_id, departure_time);

  // search the valid route
  const route* found_route = nullptr;
  if( routes.size() == 1 ) {
    found_route = &routes.at(0);
  } else {
    for( const route& route_ : routes ) {
      for( const route_entry& route_entry_ : route_ ) {
        if( std::get<2>(route_entry_) == nullptr )
          break;
        //if( route_entry_.second->d_time > departure_time )
        //  break;

        if( std::get<0>(route_entry_)->_id == station_id ) {
          if( std::get<2>(route_entry_)->d_time == departure_time ) {
            std::cout << "identified route" << std::endl;
            found_route = &route_;
            goto search_for_route_end;
          }
        }
      }
    }
  }
search_for_route_end:
  if( found_route != nullptr )
  {
    std::vector<flatbuffers::Offset<RailViz_route_entry>> fb_route_array;
    msg_ptr msg = make_route_at_time_msg( lock.sched(), *found_route );
    return cb(msg, boost::system::error_code());
  }


  // ERROR HANDLING
  flatbuffers::FlatBufferBuilder b;
  MessageBuilder msgBuilder(b);
  MotisErrorBuilder errorBuilder(b);
  errorBuilder.add_reason(b.CreateString("route not found"));
  msgBuilder.add_content_type( MsgContent_MotisError );
  msgBuilder.add_content( errorBuilder.Finish().Union() );

  b.Finish(msgBuilder.Finish());
  b.Finish(CreateMessage(b, MsgContent_MotisError, errorBuilder.Finish().Union()));
  return cb(make_msg(b), boost::system::error_code());
}

msg_ptr railviz::make_route_at_time_msg(const motis::schedule& sched, const route& route_ ) const
{
  flatbuffers::FlatBufferBuilder b;
  auto& stations = sched.stations;

  std::vector<flatbuffers::Offset<RailViz_route_entry>> fb_route_offsets;
  for( auto it = route_.begin(); it != route_.end(); ++it ) {
    unsigned int departure_id = std::get<0>(*it)->_id;
    motis::string station_name = stations[departure_id].get()->name;
    RailViz_route_entryBuilder entryBuilder(b);
    entryBuilder.add_departure_id(departure_id);
    entryBuilder.add_departure_station_name(b.CreateString(station_name.to_string()));
    if( it != route_.end()-1 ) {
      auto it_next = it +1;
      RailVizTrain t(std::get<2>(*it)->d_time, std::get<2>(*it)->a_time, departure_id, std::get<0>(*it_next)->_id, 0);
      entryBuilder.add_train_departure(&t);
    }
    fb_route_offsets.push_back( entryBuilder.Finish() );
  }

  RailViz_route_at_time_resBuilder resBuilder(b);
  const route_entry& first_entry = route_.at(0);
  resBuilder.add_line_name(b.CreateString(std::get<2>(first_entry)->_full_con->con_info->line_identifier.to_string()));
  resBuilder.add_line_type(b.CreateString(railviz::clasz_names[std::get<2>(first_entry)->_full_con->clasz]));
  resBuilder.add_route(b.CreateVector(fb_route_offsets));

  b.Finish(CreateMessage(b, MsgContent_RailViz_route_at_time_res, resBuilder.Finish().Union()));

  msg_ptr msg = make_msg(b);
  return msg;
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
  auto const& stations = lock.sched().stations;
  flatbuffers::FlatBufferBuilder b;

  std::vector<flatbuffers::Offset<RailViz_init_entry>> station_entries;
  for (auto const& station : stations) {
    RailVizCoordinate sc (station->width, station->length);
    station_entries.push_back(CreateRailViz_init_entry(
                      b, b.CreateString(stations[station->index]->name.to_string()),
                      &sc));
  }


  b.Finish(CreateMessage(
      b, MsgContent_RailViz_init,
      CreateRailViz_init(
          b, b.CreateVector(station_entries),
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
