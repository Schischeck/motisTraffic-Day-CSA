#include "motis/railviz/railviz.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/module/api.h"

#include <chrono>

using namespace json11;
using namespace motis::module;
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
    RTreePoint p1,p2;
    p1.set<0>(msg["p1"]["lat"].number_value());
    p1.set<1>(msg["p1"]["lng"].number_value());
    p2.set<0>(msg["p2"]["lat"].number_value());
    p2.set<1>(msg["p2"]["lng"].number_value());
    RTreeBox bounds(p1, p2);
    Context c = r->cmgr.createContext(bounds);
    return {Json::object{{"type", "init_context"}, {"context_id", (int)c.getID()}}};
}

std::vector<Json> all_trains( railviz* r, Json const& msg );

std::vector<Json> change_bounds( railviz* r, Json const& msg )
{
    RTreePoint p1,p2;
    p1.set<0>(msg["p1"]["lat"].number_value());
    p1.set<1>(msg["p1"]["lng"].number_value());
    p2.set<0>(msg["p2"]["lat"].number_value());
    p2.set<1>(msg["p2"]["lng"].number_value());
    RTreeBox bounds(p1, p2);
    Context& c = r->cmgr.getContext(msg["context_id"].int_value());
    c.setBounds(bounds);
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
    // trains are represented by lightconnections
    auto trains = Json::array();
    auto train_type = msg["train_class"].int_value();
    for (auto const& station_node : r->schedule_->station_nodes) {
        for (auto const& route_node : station_node->get_route_nodes()) {
            for (auto const& edge : route_node->_edges) {
                if (edge._m._type != motis::edge::ROUTE_EDGE) continue;
                motis::array<motis::light_connection> const& l_connections = edge._m._route_edge._conns;
                if (l_connections._used_size != 0 && l_connections[0]._full_con->clasz == train_type){
                    if (l_connections._used_size != 0){
                        for (motis::light_connection const& l_conn : l_connections) {
                            int dst_station = edge._to->get_station()->_id;
                            trains.push_back(Json::object{
                                                 {"dTime", tdTimeToEpochTime(r, l_conn.d_time)},
                                                 {"aTime", tdTimeToEpochTime(r, l_conn.a_time)},
                                                 {"dStation", (int) station_node.get()->_id},
                                                 {"aStation", (int) dst_station}
                                             });
                        }
                    }
                }
            }
        }
    }
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

void railviz::init()
{
    // load 10 rtrees
    for( int i = 0; i < 10; i++ )
        rtrees.push_back(RTree());
    // reorganize td-graph in rtrees
    for (auto const& station_node : this->schedule_->station_nodes) {
        motis::station* station = this->schedule_->stations[station_node.get()->_id].get();
        for (auto const& route_node : station_node->get_route_nodes()) {
            for (auto const& edge : route_node->_edges) {
                if (edge._m._type != motis::edge::ROUTE_EDGE) continue;
                auto const& l_connections = edge._m._route_edge._conns;
                if(l_connections.size() == 0) continue;
                // get Classz
                int classz = l_connections[0]._full_con->clasz;

                // calculate tracks bounding-box
                RTreePoint p1;
                p1.set<0>( station->length );
                p1.set<1>( station->width );

                motis::station* station2 = this->schedule_->stations[edge._to->get_station()->_id].get();
                RTreePoint p2;
//                motis::station* station2 = this->schedule_->stations[edge._to->get_station()->_id].get();
//                RTreePoint p2;
//                p2.set<0>( station2->length );
//                p2.set<1>( station2->width );
                p2.set<0>( station->length );
                p2.set<1>( station->width );

                RTreeBox bb(p1, p2);
                const motis::edge* edgePTR = &edge;
                RTreeValue val = std::make_pair(bb, std::make_pair(station->index,edgePTR));
                rtrees.at(classz).insert(val);
            }
        }
    }
    std::cout << "railviz initialized" << std::endl;
}

std::vector<Json> railviz::on_msg(Json const& msg, sid) {
    auto op = ops_.find(msg["type"].string_value());
    if (op == end(ops_)) {
        return {};
    }
    return op->second(this, msg);
}

MOTIS_MODULE_DEF_MODULE(railviz)

}  // namespace railviz
}  // namespace motis
