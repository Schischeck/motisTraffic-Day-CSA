#include "motis/bikesharing/bikesharing.h"

#include <iostream>
#include <numeric>
#include "boost/program_options.hpp"

#include "parser/file.h"

#include "motis/bikesharing/nextbike_initializer.h"
#include "motis/core/common/logging.h"
#include "motis/loader/util.h"

#define DATABASE_PATH "bikesharing.database_path"
#define NEXTBIKE_PATH "bikesharing.nextbike_path"

using namespace flatbuffers;
using namespace motis::logging;
using namespace motis::module;
namespace po = boost::program_options;

namespace motis {
namespace bikesharing {

bikesharing::bikesharing()
    : database_path_("bikesharing"), nextbike_path_("") {}

po::options_description bikesharing::desc() {
  po::options_description desc("bikesharing Module");
  // clang-format off
  desc.add_options()
      (DATABASE_PATH,
       po::value<std::string>(&database_path_)->default_value(database_path_),
       "Location of the Bikesharing Database (folder or ':memory:')")
      (NEXTBIKE_PATH,
       po::value<std::string>(&nextbike_path_)->default_value(nextbike_path_),
       "Where nextbike snapshots can be found (may be folder or single file)");
  // clang-format on
  return desc;
}

void bikesharing::print(std::ostream& out) const {
  out << "  " << DATABASE_PATH << ": " << database_path_ << "\n"
      << "  " << NEXTBIKE_PATH << ": " << nextbike_path_;
}

void bikesharing::init() {
  // auto merged = load_merge_nextbike();
}

void bikesharing::on_msg(msg_ptr, sid, callback cb) {
  return cb({}, boost::system::error_code());
}

}  // namespace bikesharing
}  // namespace motis
