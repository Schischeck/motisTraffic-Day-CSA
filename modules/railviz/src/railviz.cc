#include "motis/railviz/railviz.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"

#include "motis/module/api.h"

#include "motis/railviz/train_retriever.h"

using namespace json11;
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

std::vector<Json> date_bounds( railviz* r, webclient& webclient_, Json const& msg )
{
    date_converter& cnv = r->date_converter_;

    motis::date_manager::date date = r->schedule_->date_mgr.first_date();
    std::time_t from = webclient_.get_current_time();
    date = r->schedule_->date_mgr.last_date();
    date.day = date.day+1;
    std::time_t to = cnv.convert( date );

    return {Json::object{{"type", "date_bounds"}, {"start", (int)from},
                        {"end", (int)to}}};
}

std::vector<Json> all_trains( railviz* r, webclient& webclient_, Json const& msg );

std::vector<Json> change_bounds( railviz* r, webclient& webclient_, Json const& msg )
{
    std::cout << "changing bounds" << std::endl;
    geo::coord p1 = {
        msg["p1"]["lat"].number_value(),
        msg["p1"]["lng"].number_value()
    };
    geo::coord p2 = {
        msg["p2"]["lat"].number_value(),
        msg["p2"]["lng"].number_value()
    };
    geo::box bounds = {p1, p2};
    webclient_.set_bounds(bounds);
    return all_trains(r, webclient_, msg);
}

std::vector<Json> all_stations(railviz* r, webclient& webclient_, Json const& msg) {
    auto stations = Json::array();
    for (auto const& station : r->schedule_->stations) {
        stations.push_back(Json::object{{"latitude", station->width},
                                        {"longitude", station->length}});
    }
    return {Json::object{{"type", "stations"}, {"stations", stations}}};
}

std::vector<Json> station_info(railviz* r, webclient& webclient_, Json const& msg) {
    int index = msg["station_index"].int_value();
    if (index < 0 || index >= r->schedule_->stations.size()) {
        return {};
    }
    return {Json::object{
            {"type", "station_detail"},
            {"station_name", r->schedule_->stations[index]->name.to_string()}}};
}

std::vector<Json> all_trains(railviz* r, webclient& webclient_, Json const& msg) {
    auto trainsJSON = Json::array();

    // request trains for the next 5 minutes
    train_list_ptr trains = r->train_retriever_.get()->trains(
        webclient_.get_current_time(),
        webclient_.get_current_time()+(60*5),
        webclient_.get_bounds(),
        1000
    );

    std::time_t smallest_time = 0;
    auto& tp = *trains.get();
    if(tp.size() > 0)
        smallest_time = tp[0].get()->d_time;
    std::cout << "smalles-init-time: " << smallest_time << std::endl;
    for( auto &trainptr : *trains.get() )
    {
        train& t = *trainptr.get();
        if( t.d_time < smallest_time )
            smallest_time = t.d_time;
        trainsJSON.push_back(Json::object{
                             {"dTime", (int)t.d_time},
                             {"aTime", (int)t.a_time},
                             {"dStation", (int)t.d_station},
                             {"aStation", (int)t.a_station}
                         });
    }

    return {Json::object{
            {"type", "trains"}, {"server_time", (int)smallest_time}, {"trains", trainsJSON}}};
}

railviz::railviz()
    : ops_{
        {"all_stations", all_stations},
        {"station_info", station_info},
        {"all_trains", all_trains},
        {"date_bounds", date_bounds},
        {"change_bounds", change_bounds}}
{}

railviz::~railviz() {}

void railviz::init() {
  scoped_timer geo_index_timer("train retriever init");
  train_retriever_ =
      std::unique_ptr<train_retriever>(new train_retriever(*schedule_));
  date_converter_.set_date_manager( schedule_->date_mgr );
}

void railviz::on_open(sid session) {
  // Session initialization goes here.
  // TODO send initial bootstrap data like station positions
  // TODO create client context
  (*send_)(Json::object{{"hello", "world"}}, session);
}

void railviz::on_close(sid session) {
  // TODO clean up client context data
  std::cout << "Hope to see " << session << " again, soon!\n";
}

Json railviz::on_msg(Json const& msg, sid sid_) {
  if(!webclient_manager_.webclient_exists(sid_)) {
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
