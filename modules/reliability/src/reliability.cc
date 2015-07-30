#include "motis/reliability/reliability.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/module/api.h"

#include "motis/reliability/error.h"

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

void reliability::on_msg(msg_ptr msg, sid, callback cb) {
  return cb({}, error::not_implemented);
}

MOTIS_MODULE_DEF_MODULE(reliability)

}  // namespace reliability
}  // namespace motis
