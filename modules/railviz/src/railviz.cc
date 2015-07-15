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
    std::time_t from = cnv.convert( date );
    date = r->schedule_->date_mgr.last_date();
    date.day = date.day+1;
    std::time_t to = cnv.convert( date );

    return {Json::object{{"type", "date_bounds"}, {"start", (int)from},
                        {"end", (int)to}}};
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

    geo::coord p1 = {
        msg["p1"]["lat"].number_value(),
        msg["p1"]["lng"].number_value()
    };
    geo::coord p2 = {
        msg["p2"]["lat"].number_value(),
        msg["p2"]["lng"].number_value()
    };
    geo::box bounds = {p1, p2};
    webclient_.bounds = bounds;
    webclient_.time = msg["time"].number_value();

    // request trains for the next 5 minutes
    train_retriever::train_vector trains = r->train_retriever_.get()->trains(
        r->date_converter_.convert_to_motis(webclient_.time),
        r->date_converter_.convert_to_motis(webclient_.time+(60*5)),
        webclient_.bounds,
        1000
    );

    for( train_retriever::train_pair& trainp : trains )
    {
        const motis::edge* e = trainp.first;
        const motis::light_connection* lc = trainp.second;

        std::cout << "atime: " << lc->a_time << std::endl;
        std::cout << "dtime: " << lc->d_time << std::endl << std::endl;

        trainsJSON.push_back(Json::object{
                                 {"dTime", (int)r->date_converter_.convert(lc->d_time)},
                                 {"aTime", (int)r->date_converter_.convert(lc->a_time)},
                                 {"dStation", (int)e->_from->get_station()->_id},
                                 {"aStation", (int)e->_to->get_station()->_id}
                             });
    }

    std::cout << "trains: " << trains.size() << std::endl;
    return {Json::object{
            {"type", "trains"}, {"trains", trainsJSON}}};
}

railviz::railviz()
    : ops_{
        {"all_stations", all_stations},
        {"station_info", station_info},
        {"all_trains", all_trains},
        {"date_bounds", date_bounds}}
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

  if(webclient_manager_.webclient_exists(session))
      webclient_manager_.remove_webclient(session);
  webclient_manager_.create_webclient(session);
  (*send_)(Json::object{{"hello", "world"}}, session);
}

void railviz::on_close(sid session) {
  // TODO clean up client context data
  if(webclient_manager_.webclient_exists(session))
      webclient_manager_.remove_webclient(session);
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
