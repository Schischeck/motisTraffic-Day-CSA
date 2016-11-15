#include "motis/routes/prepare/seq/osrm_routing.h"

#include "osrm/engine_config.hpp"
#include "osrm/multi_target_parameters.hpp"
#include "osrm/osrm.hpp"
#include "osrm/route_parameters.hpp"
#include "osrm/smooth_via_parameters.hpp"

#include "util/coordinate.hpp"
#include "util/json_container.hpp"
#include "util/json_util.hpp"

#include "geo/latlng.h"

#include "motis/core/common/transform_to_vec.h"

using namespace osrm;
using namespace osrm::util;
using namespace osrm::util::json;

namespace motis {
namespace routes {

struct cost {
  cost(){};
  cost(int distance, int duration) : distance_(distance), duration_(duration){};

  friend bool operator<(cost const& lhs, cost const& rhs) {
    return std::tie(lhs.distance_, lhs.duration_) <
           std::tie(rhs.distance_, rhs.duration_);
  }

  friend bool operator==(cost const& lhs, cost const& rhs) {
    return std::tie(lhs.distance_, lhs.duration_) ==
           std::tie(rhs.distance_, rhs.duration_);
  }

  int distance_;
  int duration_;
};

struct osrm_routing::impl {
  impl(std::string path) {
    EngineConfig config;
    config.storage_config = {path};
    config.use_shared_memory = false;

    osrm_ = std::make_unique<OSRM>(config);
  }

  FloatCoordinate make_coord(geo::latlng l) {
    return FloatCoordinate{FloatLongitude{l.lng_}, FloatLatitude{l.lat_}};
  }

  std::vector<cost> one_to_many(geo::latlng const& one,
                                std::vector<geo::latlng> const& many) {
    MultiTargetParameters params;
    params.forward = true;  // ??

    params.coordinates.push_back(make_coord(one));

    for (auto const& m : many) {
      params.coordinates.push_back(make_coord(m));
    }

    Object result;
    auto const status = osrm_->MultiTarget(params, result);
    if (status != Status::Ok) {
      return {};
    }

    std::vector<cost> costs;
    for (auto const& cost : result.values["costs"].get<Array>().values) {
      auto const& cost_obj = cost.get<Object>();
      costs.emplace_back(cost_obj.values.at("duration").get<Number>().value,
                         cost_obj.values.at("distance").get<Number>().value);
    }
    return costs;
  }

  std::vector<routing_result> route(geo::latlng start, geo::latlng end) {
    RouteParameters params;
    params.geometries = RouteParameters::GeometriesType::CoordVec1D;
    params.overview = RouteParameters::OverviewType::Full;

    params.coordinates.push_back(make_coord(start));
    params.coordinates.push_back(make_coord(end));

    Object result;
    auto const status = osrm_->Route(params, result);

    if (status != Status::Ok) {
      return {};
    }

    auto& all_routes = result.values["routes"];
    if (all_routes.get<Array>().values.empty()) {
      return {};
    }
    std::vector<routing_result> results;
    for (auto const& r : all_routes.get<Array>().values) {
      auto const& route = r.get<Object>();
      auto const& points = transform_to_vec(
          route.values.at("geometry").get<Array>().values,
          [](auto&& jc) { return jc.template get<Number>().value; });

      std::vector<geo::latlng> polyline;
      for (auto i = 0u; i < points.size() - 2; i = i + 2) {
        polyline.emplace_back(points[i], points[i + 1]);
      }

      results.emplace_back(polyline,
                           source_spec(0, source_spec::category::UNKNOWN,
                                       source_spec::type::OSRM),
                           route.values.at("distance").get<Number>().value);
    }
    return results;
  }

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& to) {
    return {};
  }

  std::vector<node_ref> close_nodes(std::string const& station_id) {
    return {};
  }

  std::unique_ptr<OSRM> osrm_;
};

osrm_routing::osrm_routing(std::string path)
    : impl_(std::make_unique<osrm_routing::impl>(std::move(path))) {}
osrm_routing::~osrm_routing() = default;

std::vector<std::vector<routing_result>> osrm_routing::find_routes(
    std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
  return impl_->find_routes(to);
}

std::vector<node_ref> osrm_routing::close_nodes(std::string const& station_id) {
  return impl_->close_nodes(station_id);
}

}  // namespace routes
}  // namespace motis
