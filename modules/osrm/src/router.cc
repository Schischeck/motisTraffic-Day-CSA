#include "motis/osrm/router.h"

#include "osrm-backend/library/osrm.hpp"
#include "osrm-backend/plugins/multi_target.hpp"
#include "osrm-backend/plugins/polyline.hpp"
#include "osrm-backend/server/data_structures/datafacade_base.hpp"
#include "osrm-backend/server/data_structures/internal_datafacade.hpp"
#include "osrm-backend/util/routed_options.hpp"
#include "osrm-backend/util/simple_logger.hpp"

#include "motis/osrm/error.h"

using namespace motis::module;

namespace motis {
namespace osrm {

struct router::impl {
public:
  typedef InternalDataFacade<QueryEdge::EdgeData> data_facade_impl;
  typedef BaseDataFacade<QueryEdge::EdgeData> my_data_facade;
  typedef MultiTargetPlugin<my_data_facade, true> multi_target;
  typedef MultiTargetPlugin<my_data_facade, false> multi_source;
  typedef PolylinePlugin<data_facade_impl> polyline_plugin;

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
    polyline_.reset(new polyline_plugin(query_data_facade_.get()));
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

  msg_ptr polyline(OSRMPolylineRequest const* req) {
    RouteParameters params;
    params.check_sum = UINT_MAX;
    params.print_instructions = false;
    params.geometry = true;
    params.compression = false;

    params.addCoordinate({req->from()->lat(), req->from()->lng()});
    params.addCoordinate({req->to()->lat(), req->to()->lng()});

    std::vector<FixedPointCoordinate> result;
    int error = polyline_->HandleRequest(params, result);

    if (error != 200) {
      throw std::system_error(error::no_routing_response);
    }

    std::vector<double> polyline;
    for(auto const& coord : result) {
      polyline.push_back(coord.lat / COORDINATE_PRECISION);
      polyline.push_back(coord.lon / COORDINATE_PRECISION);
    }

    message_creator mc;
    mc.create_and_finish(
        MsgContent_OSRMPolylineResponse,
        CreateOSRMPolylineResponse(mc, mc.CreateVector(polyline)).Union());
    return make_msg(mc);
  }

  std::unique_ptr<data_facade_impl> query_data_facade_;
  std::unique_ptr<multi_target> multi_target_;
  std::unique_ptr<multi_source> multi_source_;
  std::unique_ptr<polyline_plugin> polyline_;
};

router::router(std::string path)
    : impl_(std::make_unique<router::impl>(path)){};
router::~router() = default;

msg_ptr router::one_to_many(OSRMOneToManyRequest const* req) const {
  return impl_->one_to_many(req);
}

msg_ptr router::polyline(OSRMPolylineRequest const* req) const {
  return impl_->polyline(req);
}

}  // namespace osrm
}  // namespace motis
