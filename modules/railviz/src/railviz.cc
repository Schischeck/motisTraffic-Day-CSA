#include "motis/railviz/railviz.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/schedule/time.h"

#include "motis/protocol/RailVizInit_generated.h"
#include "motis/protocol/RailVizAlltraReq_generated.h"
#include "motis/protocol/RailVizAlltraRes_generated.h"
#include "motis/protocol/RailVizStationDetailReq_generated.h"
#include "motis/protocol/RailVizStationDetailRes_generated.h"
#include "motis/protocol/RailVizRouteAtTimeReq_generated.h"
#include "motis/protocol/RailVizRouteAtTimeRes_generated.h"
#include "motis/protocol/RailVizTrain_generated.h"
#include "motis/protocol/RealtimeTrainInfoRequest_generated.h"

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

void railviz::print(std::ostream&) const {}

std::vector<std::string> railviz::clasz_names = {
    "ICE", "IC", "N", "RE", "RB", "S", "U", "STR", "BUS", "X"};

railviz::railviz()
    : ops_{{MsgContent_RailVizAlltraReq,
            std::bind(&railviz::all_trains, this, p::_1, p::_2, p::_3)},
           {MsgContent_RailVizStationDetailReq,
            std::bind(&railviz::station_info, this, p::_1, p::_2, p::_3)},
           {MsgContent_RailVizRouteAtTimeReq,
            std::bind(&railviz::route_at_time, this, p::_1, p::_2, p::_3)},
           {MsgContent_RailVizFindTrain,
            std::bind(&railviz::find_train, this, p::_1, p::_2, p::_3)}} {}

railviz::~railviz() {}

void railviz::find_train(msg_ptr msg, webclient& client, callback cb) {
  auto req = msg->content<RailVizFindTrain const*>();

  MessageCreator b;
  std::string search_string;
  search_string.assign(req->train_number()->c_str());

  if (search_string == "") {
    return cb({}, error::train_not_found);
  }

  for (int i = 0; i <= 10; ++i) {
    std::pair<light_connection const*, edge const*> train;
    {
      auto lock = synced_sched<schedule_access::RO>();
      train = train_retriever_->search_train(
          search_string, unix_to_motistime(schedule_begin_, client.time),
          unix_to_motistime(schedule_begin_, client.time + (60 * 5)), i);
    }
    if (train.first != nullptr) {
      b.CreateAndFinish(
          MsgContent_RailVizRouteAtTimeReq,
          CreateRailVizRouteAtTimeReq(
              b, train.second->_from->get_station()->_id,
              motis_to_unixtime(schedule_begin_, train.first->d_time),
              train.second->_from->_route)
              .Union());
      return route_at_time(make_msg(b), client, cb);
    }
  }

  return cb({}, error::train_not_found);
}

void railviz::all_trains(msg_ptr msg, webclient& client, callback cb) {
  auto lock = synced_sched<schedule_access::RO>();
  auto req = msg->content<RailVizAlltraReq const*>();

  client.bounds = {{req->p1()->lat(), req->p1()->lng()},
                   {req->p2()->lat(), req->p2()->lng()}};
  client.time = req->time();
  // request trains for the next 5 minutes
  auto trains = train_retriever_->trains(
      unix_to_motistime(schedule_begin_, client.time),
      unix_to_motistime(schedule_begin_, client.time + (60 * 5)), 1000,
      client.bounds);

  auto request_msg = make_all_trains_realtime_request(trains);
  callback callback_ =
      make_all_trains_realtime_callback(trains, req->with_routes(), cb);

  dispatch(request_msg, client.id, callback_);
}

motis::module::msg_ptr railviz::make_all_trains_realtime_request(
    std::vector<std::pair<light_connection const*, edge const*>> const& trains)
    const {
  using namespace realtime;
  MessageCreator b;
  std::vector<flatbuffers::Offset<RealtimeTrainInfoRequest>> train_requests;

  for (auto const& t : trains) {
    light_connection const* con;
    edge const* e;
    std::tie(con, e) = t;

    train_requests.push_back(CreateRealtimeTrainInfoRequest(
        b, CreateGraphTrainEvent(b, con->_full_con->con_info->train_nr,
                                 e->_from->get_station()->_id, true,
                                 con->d_time, e->_from->_route),
        true));
    train_requests.push_back(CreateRealtimeTrainInfoRequest(
        b, CreateGraphTrainEvent(b, con->_full_con->con_info->train_nr,
                                 e->_to->get_station()->_id, false, con->a_time,
                                 e->_from->_route),
        true));
  }

  b.CreateAndFinish(
      MsgContent_RealtimeTrainInfoBatchRequest,
      CreateRealtimeTrainInfoBatchRequest(b, b.CreateVector(train_requests))
          .Union());
  return make_msg(b);
}

callback railviz::make_all_trains_realtime_callback(
    std::vector<std::pair<light_connection const*, edge const*>> const& trains,
    bool with_routes, callback cb) {
  return [this, trains, cb, with_routes](msg_ptr msg,
                                         boost::system::error_code) mutable {
    auto lock = synced_sched<schedule_access::RO>();

    MessageCreator b;
    realtime_response realtime_response_(msg);
    std::vector<flatbuffers::Offset<RailVizTrain>> trains_output;
    std::vector<flatbuffers::Offset<RailVizAlltraResRoute>> fb_routes;
    for (auto const& t : trains) {
      light_connection const* con;
      edge const* e;
      std::tie(con, e) = t;
      auto delays = realtime_response_.delay(*con, *e);

      trains_output.push_back(CreateRailVizTrain(
          b, motis_to_unixtime(schedule_begin_, con->d_time),
          motis_to_unixtime(schedule_begin_, con->a_time),
          e->_from->get_station()->_id, e->_to->get_station()->_id,
          e->_from->_route, delays.first, delays.second));

      if (with_routes) {
        std::vector<unsigned int> fb_route;
        std::vector<const motis::station_node*> stations =
            timetable_retriever_.stations_on_route(*e->_from);
        for (const motis::station_node* station_node_ : stations) {
          fb_route.push_back(station_node_->_id);
        }
        fb_routes.push_back(
            CreateRailVizAlltraResRoute(b, b.CreateVector(fb_route)));
      }
    }

    b.CreateAndFinish(MsgContent_RailVizAlltraRes,
                      CreateRailVizAlltraRes(b, b.CreateVector(trains_output),
                                             b.CreateVector(fb_routes))
                          .Union());
    cb(make_msg(b), {});
  };
}

void railviz::station_info(msg_ptr msg, webclient& client, callback cb) {
  auto lock = synced_sched<schedule_access::RO>();
  auto const& stations = lock.sched().stations;
  auto const& station_nodes = lock.sched().station_nodes;
  MessageCreator b;

  auto req = msg->content<RailVizStationDetailReq const*>();
  unsigned index = req->station_index();

  if (index >= stations.size()) {
    return cb({}, error::station_index_out_of_bounds);
  }

  timetable timetable_ = timetable_retriever_.ordered_timetable_for_station(
      *station_nodes[index].get());

  msg_ptr realtime_msg = make_station_info_realtime_request(timetable_);
  callback realtime_cb =
      make_station_info_realtime_callback(index, timetable_, cb);

  return dispatch(realtime_msg, client.id, realtime_cb);
}

motis::module::msg_ptr railviz::make_station_info_realtime_request(
    const timetable& timetable_) const {
  using namespace realtime;

  MessageCreator b;
  std::vector<flatbuffers::Offset<RealtimeTrainInfoRequest>> train_requests;

  for (auto const& te : timetable_) {
    light_connection const* con = std::get<0>(te);
    station_node const* station = std::get<1>(te);
    bool departure = std::get<4>(te);
    int route_id = std::get<5>(te);
    train_requests.push_back(CreateRealtimeTrainInfoRequest(
        b, CreateGraphTrainEvent(
               b, con->_full_con->con_info->train_nr, station->_id, departure,
               departure ? con->d_time : con->a_time, route_id),
        true));
  }

  b.CreateAndFinish(
      MsgContent_RealtimeTrainInfoBatchRequest,
      CreateRealtimeTrainInfoBatchRequest(b, b.CreateVector(train_requests))
          .Union());
  return make_msg(b);
}

callback railviz::make_station_info_realtime_callback(
    int station_index, timetable const& timetable_, callback cb) {
  return [this, timetable_, station_index, cb](msg_ptr msg,
                                               boost::system::error_code) {
    auto lock = synced_sched<schedule_access::RO>();
    auto const& stations = lock.sched().stations;
    MessageCreator b;

    realtime_response realtime_response_(msg);

    std::vector<flatbuffers::Offset<RailVizStationDetailResEntry>> timetable_fb;
    for (const timetable_entry& entry : timetable_) {
      const light_connection* lc = std::get<0>(entry);
      const station_node* current_station = std::get<1>(entry);
      const station_node* end_start_station = std::get<3>(entry);
      const station_node* prev_next_station = std::get<2>(entry);
      bool outgoing = std::get<4>(entry);
      unsigned int route = std::get<5>(entry);

      int classz = lc->_full_con->clasz;
      std::string category_string =
          lock.sched().categories[lc->_full_con->con_info->family]->name;
      std::string train_num =
          category_string + std::to_string(lc->_full_con->con_info->train_nr);
      std::string train_identifier =
          category_string + lc->_full_con->con_info->line_identifier;

      std::string end_station_name =
          stations[end_start_station->_id].get()->name;

      std::time_t a_time = motis_to_unixtime(schedule_begin_, lc->a_time);
      std::time_t d_time = motis_to_unixtime(schedule_begin_, lc->d_time);
      unsigned int a_station = 0;
      unsigned int d_station = 0;
      unsigned int d_delay = 0;
      unsigned int a_delay = 0;
      unsigned int delay = realtime_response_.delay(entry);
      if (std::get<4>(entry)) {
        d_delay = delay;
        d_station = current_station->_id;
        a_station = prev_next_station->_id;
      } else {
        a_delay = delay;
        d_station = prev_next_station->_id;
        a_station = current_station->_id;
      }

      const flatbuffers::Offset<RailVizTrain>& t = CreateRailVizTrain(
          b, d_time, a_time, d_station, a_station, route, d_delay, a_delay);
      timetable_fb.push_back(CreateRailVizStationDetailResEntry(
          b, b.CreateString(train_num), b.CreateString(train_identifier),
          classz, t, b.CreateString(end_station_name), end_start_station->_id,
          outgoing));
    }
    b.CreateAndFinish(MsgContent_RailVizStationDetailRes,
                      CreateRailVizStationDetailRes(
                          b, b.CreateString(stations[station_index]->name),
                          station_index, b.CreateVector(timetable_fb))
                          .Union());
    return cb(make_msg(b), boost::system::error_code());
  };
}

void railviz::route_at_time(msg_ptr msg, webclient& client, callback cb) {
  auto lock = synced_sched<schedule_access::RO>();
  auto const& station_nodes = lock.sched().station_nodes;
  MessageCreator b;

  RailVizRouteAtTimeReq const* req =
      msg->content<RailVizRouteAtTimeReq const*>();
  unsigned int station_id = req->station_id();
  motis::time departure_time =
      unix_to_motistime(schedule_begin_, req->departure_time());
  unsigned int route_id = req->route_id();
  std::vector<route> routes;
  for (auto const& route_node :
       station_nodes[station_id].get()->get_route_nodes()) {
    if (static_cast<unsigned>(route_node->_route) == route_id)
      routes =
          timetable_retriever_.get_routes_on_time(*route_node, departure_time);
  }
  // search the valid route
  const route* found_route = nullptr;
  if (routes.size() == 1) {
    found_route = &routes.at(0);
  } else {
    for (const route& route_ : routes) {
      for (const route_entry& route_entry_ : route_) {
        if (std::get<2>(route_entry_) == nullptr) break;

        if (std::get<0>(route_entry_)->_id == station_id) {
          if (std::get<2>(route_entry_)->d_time == departure_time) {
            found_route = &route_;
            goto search_for_route_end;
          }
        }
      }
    }
  }
search_for_route_end:
  if (found_route != nullptr) {
    const route_entry& first_route_entry = found_route->at(0);
    unsigned int train_nr =
        std::get<2>(first_route_entry)->_full_con->con_info->train_nr;
    unsigned int station_index = std::get<0>(first_route_entry)->_id;
    unsigned int real_time = std::get<2>(first_route_entry)->d_time;
    unsigned int route_id = std::get<1>(first_route_entry)->_route;
    callback callback_ = make_route_at_time_realtime_callback(*found_route, cb);
    b.CreateAndFinish(
        MsgContent_RealtimeTrainInfoRequest,
        realtime::CreateRealtimeTrainInfoRequest(
            b, realtime::CreateGraphTrainEvent(b, train_nr, station_index, true,
                                               real_time, route_id))
            .Union());
    return dispatch(make_msg(b), client.id, callback_);
  }

  return cb({}, error::route_not_found);
}

callback railviz::make_route_at_time_realtime_callback(const route& route_,
                                                       callback cb) {
  return [this, route_, cb](msg_ptr msg,
                            boost::system::error_code err) mutable {
    MessageCreator b;
    auto lock = synced_sched<schedule_access::RO>();
    auto const& stations = lock.sched().stations;

    realtime_response realtime_response_(msg);
    std::vector<flatbuffers::Offset<RailVizRouteEntry>> fb_route_offsets;
    for (auto it = route_.begin(); it != route_.end(); ++it) {
      unsigned int departure_id = std::get<0>(*it)->_id;
      auto const& station_name = stations[departure_id]->name;
      if (it != route_.end() - 1) {
        auto it_next = it + 1;
        std::pair<unsigned int, unsigned int> delays =
            realtime_response_.delay(*it);
        fb_route_offsets.push_back(CreateRailVizRouteEntry(
            b, departure_id, b.CreateString(station_name),
            CreateRailVizTrain(
                b, motis_to_unixtime(schedule_begin_, std::get<2>(*it)->d_time),
                motis_to_unixtime(schedule_begin_, std::get<2>(*it)->a_time),
                departure_id, std::get<0>(*it_next)->_id, 0, delays.first,
                delays.second)));
      } else {
        fb_route_offsets.push_back(CreateRailVizRouteEntry(
            b, departure_id, b.CreateString(station_name)));
      }
    }

    RailVizRouteAtTimeResBuilder resBuilder(b);
    const route_entry& first_entry = route_.at(0);
    const motis::light_connection* lc = std::get<2>(first_entry);
    std::string category_string =
        lock.sched().categories[lc->_full_con->con_info->family]->name;
    std::string train_num =
        category_string + std::to_string(lc->_full_con->con_info->train_nr);
    std::string train_identifier =
        category_string + lc->_full_con->con_info->line_identifier;
    int classz = lc->_full_con->clasz;

    resBuilder.add_train_num(b.CreateString(train_num));
    resBuilder.add_train_identifier(b.CreateString(train_identifier));
    resBuilder.add_classz(classz);
    resBuilder.add_route(b.CreateVector(fb_route_offsets));

    b.CreateAndFinish(MsgContent_RailVizRouteAtTimeRes,
                      resBuilder.Finish().Union());

    return cb(make_msg(b), err);
  };
}

void railviz::init() {
  auto lock = synced_sched<schedule_access::RO>();
  train_retriever_ =
      std::unique_ptr<train_retriever>(new train_retriever(lock.sched()));
	schedule_begin_ = lock.sched().schedule_begin_;
}

void railviz::on_open(sid session) {
  clients_.emplace(session, session);

  auto lock = synced_sched<schedule_access::RO>();
  auto const& stations = lock.sched().stations;
  MessageCreator b;

  std::vector<flatbuffers::Offset<RailVizInitEntry>> station_entries;
  for (auto const& station : stations) {
    RailVizCoordinate sc(station->width, station->length);
    station_entries.push_back(CreateRailVizInitEntry(
        b, b.CreateString(stations[station->index]->name), &sc));
  }
	
  b.CreateAndFinish(
      MsgContent_RailVizInit,
      CreateRailVizInit(b, b.CreateVector(station_entries),
                        lock.sched().schedule_begin_ + SCHEDULE_OFFSET,
                        lock.sched().schedule_end_ + MINUTES_A_DAY * 60)
          .Union());
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
