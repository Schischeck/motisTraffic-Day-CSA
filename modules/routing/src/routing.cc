#include "motis/routing/routing.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/module/api.h"

using namespace json11;
using namespace motis::module;
namespace po = boost::program_options;

namespace motis {
namespace routing {

po::options_description routing::desc() {
  po::options_description desc("Routing Module");
  return desc;
}

void routing::print(std::ostream& out) const {}

motis::station* get_station(Json const& json) {
}

void routing::on_msg(json11::Json const& msg, sid session, callback cb) {
}

MOTIS_MODULE_DEF_MODULE(routing)

}  // namespace routing
}  // namespace motis
