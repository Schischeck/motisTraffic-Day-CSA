#include "motis/bikesharing/bikesharing.h"

#include <iostream>
#include <numeric>

#include "boost/program_options.hpp"

#include "motis/loader/util.h"
#include "motis/protocol/Message_generated.h"

using namespace flatbuffers;
using namespace motis::module;
namespace po = boost::program_options;

namespace motis {
namespace bikesharing {

po::options_description bikesharing::desc() {
  po::options_description desc("bikesharing Module");
  return desc;
}

void bikesharing::print(std::ostream&) const {}

void bikesharing::init() {}

void bikesharing::on_msg(msg_ptr msg, sid, callback cb) {
  return cb({}, boost::system::error_code());
}

}  // namespace bikesharing
}  // namespace motis
