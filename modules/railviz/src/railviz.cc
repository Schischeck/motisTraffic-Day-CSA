#include "motis/railviz/railviz.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"

#include "motis/module/api.h"

#include "motis/railviz/edge_geo_index.h"
#include <chrono>

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


std::vector<Json> init_context( railviz* r, Json const& msg )
{
    webclient_context c = r->cmgr.create_webclient_context();
    date_converter& cnv = r->date_converter_;
    train_query q(*r->edge_geo_index_.get(), r->date_converter_);
    std::time_t first_departure = q.first_departure_time();
    c.set_time(first_departure);
    std::cout << "context created: " << c.get_id() << std::endl;
    return {Json::object{{"type", "init_context"}, {"context_id", (int)c.get_id()}}};
}

std::vector<Json> date_bounds( railviz* r, Json const& msg )
{
    webclient_context c = r->cmgr.create_webclient_context();
    date_converter& cnv = r->date_converter_;

    motis::date_manager::date date = r->schedule_->date_mgr.first_date();
    std::time_t from = c.get_current_time();
    date = r->schedule_->date_mgr.last_date();
    date.day = date.day+1;
    std::time_t to = cnv.convert( date );

    std::cout << "context: " << c.get_id() << "from: " << from << std::endl;
    return {Json::object{{"type", "date_bounds"}, {"start", (int)from},
                        {"end", (int)to}}};
}

std::vector<Json> all_trains( railviz* r, Json const& msg );

std::vector<Json> change_bounds( railviz* r, Json const& msg )
{
    std::cout << "changing bounds" << std::endl;
    geometry::point p1 = {
        msg["p1"]["lat"].number_value(),
        msg["p1"]["lng"].number_value()
    };
    geometry::point p2 = {
        msg["p2"]["lat"].number_value(),
        msg["p2"]["lng"].number_value()
    };
    geometry::box bounds = {p1, p2};
    webclient_context& c = r->cmgr.get_webclient_context(msg["context_id"].int_value());
    c.set_bounds(bounds);
    return all_trains(r, msg);
}

std::vector<Json> all_stations(railviz* r, Json const& msg) {
    auto stations = Json::array();
    for (auto const& station : r->schedule_->stations) {
        stations.push_back(Json::object{{"latitude", station->width},
                                        {"longitude", station->length}});
    }
    return {Json::object{{"type", "stations"}, {"stations", stations}}};
}

std::vector<Json> station_info(railviz* r, Json const& msg) {
    int index = msg["station_index"].int_value();
    if (index < 0 || index >= r->schedule_->stations.size()) {
        return {};
    }
    return {Json::object{
            {"type", "station_detail"},
            {"station_name", r->schedule_->stations[index]->name.to_string()}}};
}

std::vector<Json> all_trains(railviz* r, Json const& msg) {
    auto trainsJSON = Json::array();
    train_query query( *r->edge_geo_index_.get(), r->date_converter_ );

    webclient_context c = r->cmgr.get_webclient_context(msg["context_id"].int_value());
    // request trains for the next 5 minutes
    train_list_ptr trains = query.by_bounds_and_time_interval(
        c.get_bounds(),
        c.get_current_time(),
        c.get_current_time()+(60*5),
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
        {"init_context", init_context},
        {"date_bounds", date_bounds},
        {"change_bounds", change_bounds}}
{}

railviz::~railviz()
{
}

void railviz::init() {
  scoped_timer geo_index_timer("geo index init");
  edge_geo_index_ =
      std::unique_ptr<edge_geo_index>(new edge_geo_index(*schedule_));
  date_converter_.set_date_manager( schedule_->date_mgr );
}

Json railviz::on_msg(Json const& msg, sid) {
  auto op = ops_.find(msg["type"].string_value());
  if (op == end(ops_)) {
    return {};
  }
  return op->second(this, msg);
}

MOTIS_MODULE_DEF_MODULE(railviz)

}  // namespace railviz
}  // namespace motis
