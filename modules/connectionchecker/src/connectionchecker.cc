#include "motis/connectionchecker/connectionchecker.h"

#include <iostream>
#include <fstream>
#include <string>

#include "boost/program_options.hpp"

#include "motis/connectionchecker/error.h"

using namespace motis::module;
namespace po = boost::program_options;
namespace p = std::placeholders;

namespace motis {
namespace connectionchecker {

po::options_description connectionchecker::desc() {
  return {"ConnectionChecker Module"};
}

void connectionchecker::print(std::ostream&) const {}

void connectionchecker::init() {}

void connectionchecker::on_msg(msg_ptr msg, sid, callback cb) {
  return cb({}, error::not_implemented);
}

}  // namespace connectionchecker
}  // namespace motis
