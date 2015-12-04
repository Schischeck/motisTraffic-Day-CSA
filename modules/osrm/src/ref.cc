#include "osrm-backend/Library/OSRM.h"
#include "osrm-backend/plugins/multi_target.hpp"
#include "osrm-backend/Server/DataStructures/BaseDataFacade.h"
#include "osrm-backend/Server/DataStructures/InternalDataFacade.h"
#include "osrm-backend/Util/ProgramOptions.h"
#include "osrm-backend/Util/Simple_logger.hpp"

namespace imotis {

class osrm_routing::osrm_routing_impl {
public:
  typedef InternalDataFacade<QueryEdge::EdgeData> DataFacadeImpl;
  typedef BaseDataFacade<QueryEdge::EdgeData> MyDataFacade;
  typedef MultiTargetPlugin<MyDataFacade, true> MultiTarget;
  typedef MultiTargetPlugin<MyDataFacade, false> MultiSource;

  osrm_routing_impl(osrm_routing_settings const& options) {
    std::string ip_address;
    int ip_port, requested_thread_num;
    bool use_shared_memory = false, trial = false;
    ServerPaths server_paths;

    int argc = 2;
    char const* argv[] = { "./osrm-routed", options.map_path.c_str() };

    auto ret = GenerateServerProgramOptions(
      argc, argv, server_paths, ip_address, ip_port,
      requested_thread_num, use_shared_memory, trial);
    populate_base_path(server_paths);

    if (ret == INIT_FAILED) {
      throw std::runtime_error("OSRM init failed");
    }

    query_data_facade_.reset(new DataFacadeImpl(server_paths));
    multi_target_.reset(new MultiTarget(query_data_facade_.get()));
    multi_source_.reset(new MultiSource(query_data_facade_.get()));
  }

  ~osrm_routing_impl() {
  }

  void route_multi_target(std::vector<service_req_ptr>& requests) {
    return route(requests, true);
  };

  void route_multi_source(std::vector<service_req_ptr>& requests) {
    return route(requests, false);
  };

private:
  void route(std::vector<service_req_ptr>& requests, bool forward) {
    if (requests.size() == 0) {
      return;
    }

    RouteParameters params;
    params.zoom_level = 18;
    params.check_sum = UINT_MAX;

    if (forward) {
      params.coordinates.emplace_back(
        requests[0]->source.lat * COORDINATE_PRECISION,
        requests[0]->source.lng * COORDINATE_PRECISION
      );
    } else {
      params.coordinates.emplace_back(
        requests[0]->target.lat * COORDINATE_PRECISION,
        requests[0]->target.lng * COORDINATE_PRECISION
      );
    }

    for (auto const& req : requests) {
      params.coordinates.emplace_back(
        (forward ? req->target.lat : req->source.lat) * COORDINATE_PRECISION,
        (forward ? req->target.lng : req->source.lng) * COORDINATE_PRECISION
      );
    }

    unsigned calc_time_in_ms = 0;
    std::shared_ptr<std::vector<std::pair<EdgeWeight, double>>> distances;
    if (forward) {
      distances = multi_target_->HandleRequest(params, calc_time_in_ms);
    } else {
      distances = multi_source_->HandleRequest(params, calc_time_in_ms);
    }

    if (!distances) {
      return;
    }

    int i = 0;
    for (auto const& dist : *distances.get()) {
      requests[i]->success = dist.first != INT_MAX;
      requests[i]->distance = dist.second;
      requests[i]->timecost = dist.first;
      ++i;
    }
  }

  std::unique_ptr<DataFacadeImpl> query_data_facade_;
  std::unique_ptr<MultiTarget> multi_target_;
  std::unique_ptr<MultiSource> multi_source_;
};
