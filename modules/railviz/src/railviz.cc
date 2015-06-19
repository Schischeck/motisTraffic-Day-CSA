#include "motis/railviz/railviz.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/module/api.h"

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

railviz::railviz()
    : ops_{{"all_stations", all_stations}, {"station_info", station_info}} {}

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
