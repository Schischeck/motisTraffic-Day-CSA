#include "motis/bikesharing/bikesharing.h"

#include <functional>
#include <iostream>
#include <numeric>

#include "boost/program_options.hpp"

#include "parser/file.h"

#include "motis/core/common/logging.h"
#include "motis/bikesharing/database.h"
#include "motis/bikesharing/error.h"
#include "motis/bikesharing/nextbike_initializer.h"
#include "motis/bikesharing/search.h"

#define DATABASE_PATH "bikesharing.database_path"
#define NEXTBIKE_PATH "bikesharing.nextbike_path"

using namespace flatbuffers;
using namespace motis::logging;
using namespace motis::module;
namespace po = boost::program_options;
namespace p = std::placeholders;

namespace motis {
namespace bikesharing {

bikesharing::bikesharing()
    : database_path_("bikesharing"), nextbike_path_("") {}

bikesharing::~bikesharing() = default;

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

void bikesharing::init(motis::module::registry& reg) {
  reg.subscribe("/init", std::bind(&bikesharing::init_module, this, p::_1));
  reg.register_op("/bikesharing",
                  std::bind(&bikesharing::request, this, p::_1));
}

motis::module::msg_ptr bikesharing::init_module(motis::module::msg_ptr const&) {
  if (!database_path_.empty() && !nextbike_path_.empty()) {
    database_ = std::make_unique<database>(database_path_);
    initialize_nextbike(nextbike_path_, *database_);
    search_ = std::make_unique<bikesharing_search>(*database_);
  }
  return nullptr;
}

motis::module::msg_ptr bikesharing::request(motis::module::msg_ptr const& req) {
  if (!search_ || !database_) {
    throw std::system_error(error::not_initialized);
  }

  using motis::bikesharing::BikesharingRequest;
  return search_->find_connections(motis_content(BikesharingRequest, req));
}

}  // namespace bikesharing
}  // namespace motis
