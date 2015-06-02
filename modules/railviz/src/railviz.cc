#include "motis/railviz/railviz.h"

#include <iostream>

using namespace json11;
using namespace motis::module;

std::unique_ptr<motis::module::module> load_module(td::Schedule* schedule) {
  std::unique_ptr<motis::module::module> r(new motis::railviz::railviz());
  r->schedule_ = schedule;
  return r;
}

namespace motis {
namespace railviz {

std::vector<Json> all_stations(railviz* r, Json const& msg) {
  auto stations = Json::array();
  for (auto const& station : r->schedule_->stations) {
    stations.push_back(Json::object{{"latitude", station->width},
                                    {"longitude", station->length}});
  }
  return {Json::object{{"type", "stations"}, {"stations", stations}}};
};

std::vector<Json> station_info(railviz* r, Json const& msg) {
  int index = msg["station_index"].int_value();
  if (index < 0 || index >= r->schedule_->stations.size()) {
    return {};
  }
  return {Json::object{
      {"type", "station_detail"},
      {"station_name", r->schedule_->stations[index]->name.toString()}}};
};

railviz::railviz()
    : ops_{{"all_stations", all_stations}, {"station_info", station_info}} {}

std::vector<Json> railviz::on_msg(Json const& msg, sid) {
  auto op = ops_.find(msg["type"].string_value());
  if (op == end(ops_)) {
    return {};
  }
  return op->second(this, msg);
}

}  // namespace motis
}  // namespace railviz