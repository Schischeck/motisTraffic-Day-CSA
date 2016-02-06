#include "motis/bikesharing/bikesharing.h"

#include <functional>
#include <iostream>
#include <numeric>

#include "boost/program_options.hpp"

#include "parser/file.h"

#include "motis/bikesharing/error.h"
#include "motis/bikesharing/nextbike_initializer.h"
#include "motis/core/common/logging.h"
#include "motis/loader/util.h"

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

void bikesharing::init_async(callback cb) {
  database_ = make_unique<database>(database_path_);

  auto dispatch_fun = [this](msg_ptr msg, callback cb) {
    return dispatch(msg, 0ul, cb);
  };
  auto finished = [this, cb](msg_ptr msg,
                             boost::system::error_code ec) mutable {
    if (!ec) {
      search_ = make_unique<bikesharing_search>(*database_);
    }
    return cb(msg, ec);
  };
  initialize_nextbike(nextbike_path_, *database_, dispatch_fun, finished);
}

void bikesharing::on_msg(msg_ptr msg, sid, callback cb) {
  if (search_.get() == nullptr) {
    cb({}, error::not_initialized);
  }

  auto content_type = msg->content_type();
  if (content_type == MsgContent_BikesharingRequest) {
    auto req = msg->content<BikesharingRequest const*>();
    try {
      return cb(search_->find_connections(req), error::ok);
    } catch (boost::system::system_error const& e) {
      return cb({}, e.code());
    } catch (...) {
      return cb({}, error::search_failure);
    }
  }

  return cb({}, error::not_implemented);
}

}  // namespace bikesharing
}  // namespace motis
