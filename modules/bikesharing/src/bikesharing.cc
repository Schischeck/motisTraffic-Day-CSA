#include "motis/bikesharing/bikesharing.h"

#include <functional>
#include <iostream>
#include <numeric>

#include "boost/program_options.hpp"

#include "parser/file.h"

#include "motis/core/common/logging.h"
#include "motis/bikesharing/database.h"
#include "motis/bikesharing/error.h"
#include "motis/bikesharing/geo_index.h"
#include "motis/bikesharing/geo_terminals.h"
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

bikesharing::bikesharing() : module("Bikesharing Options", "bikesharing") {
  string_param(database_path_, "bikesharing", "database_path",
               "Location of the Bikesharing Database (folder or ':memory:')");
  string_param(nextbike_path_, "", "nextbike_path",
               "Where nextbike snapshots can be found (folder or single file)");
}

void bikesharing::init(motis::module::registry& reg) {
  reg.subscribe("/init", std::bind(&bikesharing::init_module, this, p::_1));
  reg.register_op("/bikesharing/search",
                  [this](msg_ptr const& req) { return search(req); });
  reg.register_op("/bikesharing/geo_terminals",
                  [this](msg_ptr const& req) { return geo_terminals(req); });
}

msg_ptr bikesharing::init_module(msg_ptr const&) {
  if (!database_path_.empty()) {
    database_ = std::make_unique<database>(database_path_);

    if (database_->is_initialized()) {
      LOG(info) << "using initialized bikesharing database";
    } else {
      if (!nextbike_path_.empty()) {
        initialize_nextbike(nextbike_path_, *database_);
      }
    }

    if (database_->is_initialized()) {
      geo_index_ = std::make_unique<geo_index>(*database_);
      search_ = std::make_unique<bikesharing_search>(*database_, *geo_index_);
    }
  }
  return nullptr;
}

msg_ptr bikesharing::search(msg_ptr const& req) const {
  ensure_initialized();

  using motis::bikesharing::BikesharingRequest;
  return search_->find_connections(motis_content(BikesharingRequest, req));
}

msg_ptr bikesharing::geo_terminals(msg_ptr const& req) const {
  ensure_initialized();

  using motis::bikesharing::BikesharingGeoTerminalsRequest;
  return motis::bikesharing::geo_terminals(
      *database_, *geo_index_,
      motis_content(BikesharingGeoTerminalsRequest, req));
}

void bikesharing::ensure_initialized() const {
  if (!database_ || !search_ || !geo_index_) {
    throw std::system_error(error::not_initialized);
  }
}

}  // namespace bikesharing
}  // namespace motis
