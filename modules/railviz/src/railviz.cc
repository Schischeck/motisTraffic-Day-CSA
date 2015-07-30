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

std::vector<Json> date_bounds(railviz* r, webclient& webclient_,
                              Json const& msg) {
  date_converter& cnv = r->date_converter_;

  motis::date_manager::date date = r->schedule_->date_mgr.first_date();
  std::time_t from = cnv.convert(date);
  date = r->schedule_->date_mgr.last_date();
  date.day = date.day + 1;
  std::time_t to = cnv.convert(date);

  return {Json::object{
      {"type", "date_bounds"}, {"start", (int)from}, {"end", (int)to}}};
}

std::vector<Json> all_stations(railviz* r, webclient& webclient_,
                               Json const& msg) {
  auto stations = Json::array();
  for (auto const& station : r->schedule_->stations) {
    stations.push_back(Json::object{{"latitude", station->width},
                                    {"longitude", station->length}});
  }
  return {Json::object{{"type", "stations"}, {"stations", stations}}};
}

std::vector<Json> station_info(railviz* r, webclient& webclient_,
                               Json const& msg) {
  int index = msg["station_index"].int_value();
  if (index < 0 || index >= r->schedule_->stations.size()) {
    return {};
  }
  return {Json::object{
      {"type", "station_detail"},
      {"station_name", r->schedule_->stations[index]->name.to_string()}}};
}

std::vector<Json> timetable_for_station(railviz* r, webclient& webclient_,
                                        Json const& msg) {
  std::cout << msg.dump() << std::endl;
  int station_id = msg["station_index"].int_value();
  time client_time = r->date_converter_.convert_to_motis(webclient_.time);
  unsigned int day_index = client_time / MINUTES_A_DAY;

  std::cout << "station_id: " << station_id << std::endl;
  std::cout << "day_index: " << day_index << std::endl;

  std::cout << "station_name: "
            << r->schedule_->stations.at(station_id).get()->name << std::endl;

  station_node& station_node_ = *r->schedule_->station_nodes[station_id].get();

  std::vector<node*> incoming_route_nodes =
      station_node_.get_incoming_route_nodes();
  std::vector<node*> outgoing_route_nodes = station_node_.get_route_nodes();

  std::cout << "found " << incoming_route_nodes.size()
            << " incoming route_nodes " << std::endl;
  std::cout << "found " << outgoing_route_nodes.size()
            << " outgoing route_nodes" << std::endl;

  Json::object json_return;
  for (node* n : outgoing_route_nodes) {
    std::cout << "outgoing route-node...";
    const station_node* dst_station_node =
        r->train_retriever_.get()->end_station_for_route(n->_route, n);
    std::string dst_name =
        r->schedule_->stations[dst_station_node->_id].get()->name;
    std::cout << "outgoing conenction to: " << dst_name << std::endl;
    std::cout << "finished" << std::endl;
  }
}

std::vector<Json> all_trains(railviz* r, webclient& webclient_,
                             Json const& msg) {
  auto trainsJSON = Json::array();

  geo::coord p1 = {msg["p1"]["lat"].number_value(),
                   msg["p1"]["lng"].number_value()};
  geo::coord p2 = {msg["p2"]["lat"].number_value(),
                   msg["p2"]["lng"].number_value()};
  geo::box bounds = {p1, p2};
  webclient_.bounds = bounds;
  webclient_.time = msg["time"].number_value();

  // request trains for the next 5 minutes
  std::vector<train> trains = r->train_retriever_.get()->trains(
      r->date_converter_.convert_to_motis(webclient_.time),
      r->date_converter_.convert_to_motis(webclient_.time + (60 * 5)),
      webclient_.bounds, 1000);

  std::cout << "sending " << trains.size() << " trains" << std::endl;

  for (train& train_ : trains) {
    // turn on to compile with -O2
    // std::cout << train_.light_conenction_->d_time << "-" <<
    // train_.light_conenction_->a_time << std::endl;
    int d_time =
        (int)r->date_converter_.convert(train_.light_conenction_->d_time);
    int a_time =
        (int)r->date_converter_.convert(train_.light_conenction_->a_time);
    // std::cout << d_time << "-" << a_time << std::endl;
    trainsJSON.push_back(Json::object{{"dTime", d_time},
                                      {"aTime", a_time},
                                      {"dStation", (int)train_.d_station},
                                      {"aStation", (int)train_.a_station},
                                      {"route_id", (int)train_.route_id}});
  }

  // std::cout << "trains: " << trains.size() << std::endl;
  return {Json::object{{"type", "trains"}, {"trains", trainsJSON}}};
}

railviz::railviz()
    : ops_{{"all_stations", all_stations},
           {"station_info", station_info},
           {"all_trains", all_trains},
           {"date_bounds", date_bounds},
           {"timetable_for_station", timetable_for_station}} {}

railviz::~railviz() {}

void railviz::init() {
  scoped_timer geo_index_timer("train retriever init");

  auto lock = synced_sched<schedule_access::RO>();
  train_retriever_ =
      std::unique_ptr<train_retriever>(new train_retriever(*schedule_));
  date_converter_.set_date_manager(schedule_->date_mgr);
}

void railviz::on_open(sid session) {
  // Session initialization goes here.
  // TODO send initial bootstrap data like station positions

  if (webclient_manager_.webclient_exists(session))
    webclient_manager_.remove_webclient(session);
  webclient_manager_.create_webclient(session);
  (*send_)(Json::object{{"hello", "world"}}, session);
}

void railviz::on_close(sid session) {
  // TODO clean up client context data
  if (webclient_manager_.webclient_exists(session))
    webclient_manager_.remove_webclient(session);
  std::cout << "Hope to see " << session << " again, soon!\n";
}

Json railviz::on_msg(Json const& msg, sid sid_) {
  if (!webclient_manager_.webclient_exists(sid_)) {
    return {"error"};
  }
  auto op = ops_.find(msg["type"].string_value());
  if (op == end(ops_)) {
    return {};
  }
  webclient& webclient_ = webclient_manager_.get_webclient(sid_);
  return op->second(this, webclient_, msg);
}

MOTIS_MODULE_DEF_MODULE(railviz)

}  // namespace railviz
}  // namespace motis
