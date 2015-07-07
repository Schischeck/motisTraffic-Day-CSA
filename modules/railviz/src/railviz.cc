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

Json all_stations(railviz* r, Json const& msg) {
  auto stations = Json::array();
  for (auto const& station : r->schedule_->stations) {
    stations.push_back(Json::object{{"latitude", station->width},
                                    {"longitude", station->length}});
  }
  return {Json::object{{"type", "stations"}, {"stations", stations}}};
}

Json station_info(railviz* r, Json const& msg) {
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

railviz::~railviz() {}

void railviz::init() {
  scoped_timer geo_index_timer("train retriever init");
  train_retriever_ =
      std::unique_ptr<train_retriever>(new train_retriever(*schedule_));
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
