#include "../include/motis/osrm/osrm.h"

#include <iostream>

#include "boost/program_options.hpp"

#include "osrm-backend/library/osrm.hpp"
#include "osrm-backend/util/routed_options.hpp"
#include "osrm-backend/plugins/multi_target.hpp"
#include "osrm-backend/server/data_structures/datafacade_base.hpp"
#include "osrm-backend/server/data_structures/internal_datafacade.hpp"
#include "osrm-backend/util/simple_logger.hpp"

#include "motis/core/common/util.h"
#include "motis/core/common/logging.h"
#include "motis/protocol/Message_generated.h"
#include "motis/osrm/error.h"

#define OSRM_DATASET_PATH "osrm.dataset_path"

using namespace flatbuffers;
using namespace motis::module;
using namespace motis::logging;
namespace po = boost::program_options;

namespace motis {
namespace osrm {

class osrm::impl {
public:
  typedef InternalDataFacade<QueryEdge::EdgeData> DataFacadeImpl;
  typedef BaseDataFacade<QueryEdge::EdgeData> MyDataFacade;
  typedef MultiTargetPlugin<MyDataFacade, true> MultiTarget;
  typedef MultiTargetPlugin<MyDataFacade, false> MultiSource;

  impl(std::string const& path) {
    std::string ip_address;
    int ip_port, requested_thread_num;
    bool use_shared_memory = false, trial = false;
    ServerPaths server_paths;
    int max_locations_distance_table, max_locations_map_matching;

    int argc = 2;
    char const* argv[] = {"./osrm-routed", path.c_str()};

    auto ret = GenerateServerProgramOptions(
        argc, argv, server_paths, ip_address, ip_port, requested_thread_num,
        use_shared_memory, trial, max_locations_distance_table,
        max_locations_map_matching);
    populate_base_path(server_paths);

    if (ret == INIT_FAILED) {
      LOG(motis::logging::error) << "OSRM initialization failed";
      return;
    }

    query_data_facade_.reset(new DataFacadeImpl(server_paths));
    multi_target_.reset(new MultiTarget(query_data_facade_.get()));
    multi_source_.reset(new MultiSource(query_data_facade_.get()));
  }

  void route(OSRMRoutingRequest const* req, callback cb) {
    RouteParameters params;
    params.zoom_level = 18;
    params.check_sum = UINT_MAX;

    params.coordinates.emplace_back(req->one()->lat() * COORDINATE_PRECISION,
                                    req->one()->lng() * COORDINATE_PRECISION);

    for (auto const& loc : *req->many()) {
      params.coordinates.emplace_back(loc->lat() * COORDINATE_PRECISION,
                                      loc->lng() * COORDINATE_PRECISION);
    }

    unsigned calc_time_in_ms = 0;
    std::shared_ptr<std::vector<std::pair<EdgeWeight, double>>> distances;
    if (req->direction() == Direction_Forward) {
      distances = multi_target_->HandleRequest(params, calc_time_in_ms);
    } else {
      distances = multi_source_->HandleRequest(params, calc_time_in_ms);
    }

    if (!distances) {
      return cb({}, error::no_routing_response);
    }

    std::vector<Cost> costs;
    std::transform(begin(*distances), end(*distances),
                   std::back_inserter(costs),
                   [](std::pair<EdgeWeight, double> const& p) {
                     return Cost(p.first, p.second);
                   });

    MessageCreator fbb;
    fbb.CreateAndFinish(
        MsgContent_OSRMRoutingResponse,
        CreateOSRMRoutingResponse(fbb, fbb.CreateVectorOfStructs(costs))
            .Union());
    return cb(make_msg(fbb), error::ok);
  }

  std::unique_ptr<DataFacadeImpl> query_data_facade_;
  std::unique_ptr<MultiTarget> multi_target_;
  std::unique_ptr<MultiSource> multi_source_;
};

osrm::osrm() {}
osrm::~osrm() {}

po::options_description osrm::desc() {
  po::options_description desc("OSRM Module");
  // clang-format off
  desc.add_options()
      (OSRM_DATASET_PATH,
       po::value<std::string>(&path_)->default_value(path_),
       "OSRM dataset path");
  // clang-format on
  return desc;
}

void osrm::print(std::ostream& out) const {
  out << "  " << OSRM_DATASET_PATH << ": " << path_;
}

void osrm::init() { impl_ = make_unique<osrm::impl>(path_); }

void osrm::on_msg(msg_ptr msg, sid, callback cb) {
  return impl_->route(msg->content<OSRMRoutingRequest const*>(), cb);
}

}  // namespace osrm
}  // namespace motis
