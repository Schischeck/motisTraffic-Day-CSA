#include "motis/reliability/reliability.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/module/api.h"

using namespace motis::module;
namespace po = boost::program_options;

namespace motis {
namespace reliability {

po::options_description reliability::desc() {
  po::options_description desc("Reliability Module");
  return desc;
}

void reliability::print(std::ostream&) const {}

reliability::reliability() {}

msg_ptr reliability::on_msg(msg_ptr const& msg, sid) { return {}; }

MOTIS_MODULE_DEF_MODULE(reliability)

}  // namespace reliability
}  // namespace motis
