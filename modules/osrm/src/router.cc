#include "motis/osrm/router.h"

#include "osrm-backend/library/osrm.hpp"
#include "osrm-backend/plugins/custom_viaroute.hpp"
#include "osrm-backend/plugins/multi_target.hpp"
#include "osrm-backend/server/data_structures/datafacade_base.hpp"
#include "osrm-backend/server/data_structures/internal_datafacade.hpp"
#include "osrm-backend/util/routed_options.hpp"
#include "osrm-backend/util/simple_logger.hpp"

#include "motis/osrm/error.h"
#include "motis/osrm/smooth_via_router.h"

using namespace flatbuffers;
using namespace motis::module;

namespace motis {
namespace osrm {

struct router::impl {
public:
  typedef InternalDataFacade<QueryEdge::EdgeData> data_facade_impl;

  typedef MultiTargetPlugin<data_facade_impl, true> multi_target;
  typedef MultiTargetPlugin<data_facade_impl, false> multi_source;
  typedef CustomViaRoutePlugin<data_facade_impl> via_route;

  explicit impl(std::string const& path) {
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
      throw std::runtime_error("OSRM initialization failed");
      return;
    }

    query_data_facade_.reset(new data_facade_impl(server_paths));
    multi_target_.reset(new multi_target(query_data_facade_.get()));
    multi_source_.reset(new multi_source(query_data_facade_.get()));
    via_route_.reset(new via_route(query_data_facade_.get()));

    smooth_via_router_.reset(
        new smooth_via_router<data_facade_impl>(query_data_facade_.get()));
  }

  msg_ptr one_to_many(OSRMOneToManyRequest const* req) {
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
      throw std::system_error(error::no_routing_response);
    }

    std::vector<Cost> costs;
    std::transform(begin(*distances), end(*distances),
                   std::back_inserter(costs),
                   [](std::pair<EdgeWeight, double> const& p) {
                     return Cost(p.first, p.second);
                   });

    message_creator fbb;
    fbb.create_and_finish(
        MsgContent_OSRMOneToManyResponse,
        CreateOSRMOneToManyResponse(fbb, fbb.CreateVectorOfStructs(costs))
            .Union());
    return make_msg(fbb);
  }

  msg_ptr via(OSRMViaRouteRequest const* req) {
    RouteParameters params;
    params.check_sum = UINT_MAX;
    params.print_instructions = false;
    params.geometry = true;
    params.compression = false;

    for (auto const& waypoint : *req->waypoints()) {
      params.addCoordinate({waypoint->lat(), waypoint->lng()});
    }

    CustomViaRouteResult result;
    int error = via_route_->HandleRequest(params, result);

    if (error != 200) {
      throw std::system_error(error::no_routing_response);
    }

    message_creator mc;
    std::vector<Offset<Polyline>> segments;
    for (auto const& raw_polyline : result.polylines) {
      std::vector<double> polyline;
      for (auto const& coord : raw_polyline) {
        polyline.push_back(coord.lat / COORDINATE_PRECISION);
        polyline.push_back(coord.lon / COORDINATE_PRECISION);
      }
      segments.push_back(CreatePolyline(mc, mc.CreateVector(polyline)));
    }

    mc.create_and_finish(
        MsgContent_OSRMViaRouteResponse,
        CreateOSRMViaRouteResponse(mc, result.time, result.distance,
                                   mc.CreateVector(segments))
            .Union());
    return make_msg(mc);
  }

  msg_ptr smooth_via(OSRMSmoothViaRouteRequest const* req) {
    return smooth_via_router_->smooth_via(req);
  }

  std::unique_ptr<data_facade_impl> query_data_facade_;
  std::unique_ptr<multi_target> multi_target_;
  std::unique_ptr<multi_source> multi_source_;
  std::unique_ptr<via_route> via_route_;

  std::unique_ptr<smooth_via_router<data_facade_impl>> smooth_via_router_;
};

router::router(std::string path)
    : impl_(std::make_unique<router::impl>(path)){};
router::~router() = default;

msg_ptr router::one_to_many(OSRMOneToManyRequest const* req) const {
  return impl_->one_to_many(req);
}

msg_ptr router::via(OSRMViaRouteRequest const* req) const {
  return impl_->via(req);
}

msg_ptr router::smooth_via(OSRMSmoothViaRouteRequest const* req) const {
  return impl_->smooth_via(req);
}

}  // namespace osrm
}  // namespace motis
