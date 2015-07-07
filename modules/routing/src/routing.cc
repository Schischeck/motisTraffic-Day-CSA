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

Json routing::on_msg(Json const& msg, sid session) {
  return Json::object{};
}

MOTIS_MODULE_DEF_MODULE(routing)

}  // namespace routing
}  // namespace motis
