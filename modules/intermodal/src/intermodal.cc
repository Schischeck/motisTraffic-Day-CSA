#include "motis/intermodal/intermodal.h"

#include <iostream>
#include <numeric>

#include "boost/program_options.hpp"

#include "motis/intermodal/error.h"

using namespace motis::module;
namespace po = boost::program_options;

namespace motis {
namespace intermodal {

po::options_description intermodal::desc() {
  po::options_description desc("Intermodal Module");
  return desc;
}

void intermodal::on_msg(msg_ptr msg, sid, callback cb) {
  return cb({}, error::not_implemented);
}

}  // namespace intermodal
}  // namespace motis
