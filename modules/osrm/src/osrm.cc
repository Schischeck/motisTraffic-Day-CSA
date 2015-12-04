#include "../include/motis/osrm/osrm.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "motis/protocol/Message_generated.h"

using namespace flatbuffers;
using namespace motis::module;
namespace po = boost::program_options;

namespace motis {
namespace osrm {

po::options_description osrm::desc() {
  po::options_description desc("OSRM Module");
  return desc;
}

void osrm::print(std::ostream& out) const {}

void osrm::init() {}

void osrm::on_msg(msg_ptr msg, sid, callback cb) {}

}  // namespace osrm
}  // namespace motis
