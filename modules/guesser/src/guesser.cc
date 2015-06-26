#include "motis/guesser/guesser.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/module/api.h"

using namespace json11;
using namespace motis::module;
namespace po = boost::program_options;

namespace motis {
namespace guesser {

po::options_description guesser::desc() {
  po::options_description desc("Guesser Module");
  return desc;
}

void guesser::print(std::ostream& out) const {}

Json guesser::on_msg(Json const& msg, sid) { return "not implemented"; }

MOTIS_MODULE_DEF_MODULE(guesser)

}  // namespace guesser
}  // namespace motis
