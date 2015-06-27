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
    geometry::point p1 = {
        msg["p1"]["lat"].number_value(),
        msg["p1"]["lng"].number_value()
    };
    geometry::point p2 = {
        msg["p2"]["lat"].number_value(),
        msg["p2"]["lng"].number_value()
    };
    geometry::box bounds = {p1, p2};
    webclient_context c = r->cmgr.create_webclient_context(bounds);
    return {Json::object{{"type", "init_context"}, {"context_id", (int)c.get_id()}}};
}

std::vector<Json> all_trains( railviz* r, Json const& msg );

std::vector<Json> change_bounds( railviz* r, Json const& msg )
{
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

int getUnixTimeStamp() {
    std::chrono::seconds ms = std::chrono::duration_cast<std::chrono::seconds> (
                std::chrono::system_clock::now().time_since_epoch()
                );
    return ms.count();
}

int tdTimeToEpochTime(railviz* r, motis::time td_time) {
    // Be aware TD works with a local time (Europe/Berlin)
    int td_day_index = td_time / MINUTES_A_DAY;
    int td_minutes = td_time % MINUTES_A_DAY;
    motis::date_manager const& date_manager =  r->schedule_->date_mgr;
    int td_day = 0;
    int td_month = 0;
    int td_year = 0;
    // there is a bug/feature in backend, you can get indexofaday out of bounds!
    if (td_day_index > date_manager.get_day_index(date_manager.last_date())) {
        motis::date_manager::date const& td_date = date_manager.last_date();
        td_day = td_date.day;
        td_month = td_date.month;
        td_year = td_date.year;
        int td_index_delta = td_day_index - date_manager.get_day_index(date_manager.last_date());
        td_day =+ td_index_delta;
    } else {
        motis::date_manager::date const& td_date = date_manager.get_date(td_day_index);
        td_day = td_date.day;
        td_month = td_date.month;
        td_year = td_date.year;
    }
    std::tm* local_time_struct = new std::tm;
    local_time_struct->tm_hour = 0;
    local_time_struct->tm_min = td_minutes;
    local_time_struct->tm_sec = 0;
    local_time_struct->tm_year = td_year;
    local_time_struct->tm_mon = td_month;
    local_time_struct->tm_mday = td_day;
    // bacause TD uses Localtime, we are using std::mktime: local_time -> unixtime
    std::time_t unix_time = std::mktime(local_time_struct);
    // convert unix time in unix time stamp
    std::chrono::seconds ms = std::chrono::duration_cast<std::chrono::seconds> (
                std::chrono::system_clock::from_time_t(unix_time).time_since_epoch()
                );
    delete local_time_struct;
    return ms.count();
}

std::vector<Json> all_trains(railviz* r, Json const& msg) {
    auto trains = Json::array();


    return {Json::object{
            {"type", "trains"}, {"server_time", getUnixTimeStamp()}, {"trains", trains}}};
}

railviz::railviz()
    : ops_{
        {"all_stations", all_stations},
        {"station_info", station_info},
        {"all_trains", all_trains},
        {"init_context", init_context}}
{}

railviz::~railviz()
{
}

void railviz::init() {
  scoped_timer geo_index_timer("geo index init");
  edge_geo_index_ =
      std::unique_ptr<edge_geo_index>(new edge_geo_index(*schedule_));
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
