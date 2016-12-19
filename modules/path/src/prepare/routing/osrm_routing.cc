#include "motis/path/prepare/routing/osrm_routing.h"

#include "osrm/engine_config.hpp"
#include "osrm/multi_target_parameters.hpp"
#include "osrm/nearest_parameters.hpp"
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
namespace path {

struct osrm_routing::impl {
  explicit impl(std::size_t router_id, std::string const& path)
      : router_id_(router_id) {
    EngineConfig config;
    config.storage_config = {path};
    config.use_shared_memory = false;

    osrm_ = std::make_unique<OSRM>(config);
  }

  FloatCoordinate make_coord(geo::latlng l) {
    return FloatCoordinate{FloatLongitude{l.lng_}, FloatLatitude{l.lat_}};
  }

  std::vector<int> one_to_many(geo::latlng const& one,
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

    std::vector<int> costs;
    for (auto const& cost : result.values["costs"].get<Array>().values) {
      auto const& cost_obj = cost.get<Object>();
      costs.emplace_back(cost_obj.values.at("distance").get<Number>().value);
    }
    return costs;
  }

  geo::polyline get_polyline(node_ref const& from, node_ref const& to) {
    if (from.router_id_ != router_id_ || to.router_id_ != router_id_) {
      return {};
    }
    RouteParameters params;
    params.geometries = RouteParameters::GeometriesType::CoordVec1D;
    params.overview = RouteParameters::OverviewType::Full;

    params.coordinates.push_back(make_coord(from.coords_));
    params.coordinates.push_back(make_coord(to.coords_));

    Object result;
    auto const status = osrm_->Route(params, result);

    if (status != Status::Ok) {
      return {};
    }

    auto& all_routes = result.values["routes"];
    if (all_routes.get<Array>().values.empty()) {
      return {};
    }
    auto& route = get(all_routes, 0u);
    auto const& points = transform_to_vec(
        get(route, "geometry").get<Array>().values,
        [](auto&& jc) { return jc.template get<Number>().value; });
    std::vector<geo::latlng> polyline;
    for (auto i = 0u; i < points.size() - 1; i = i + 2) {
      polyline.emplace_back(points[i], points[i + 1]);
    }
    return polyline;
  }

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
    std::vector<std::vector<routing_result>> result;
    auto to_coords = transform_to_vec(begin(to), end(to),
                                      [&](auto const& t) { return t.coords_; });
    for (auto f : from) {
      if (f.router_id_ != router_id_) {
        result.push_back({});
        continue;
      }
      std::vector<routing_result> from_result;
      auto costs = one_to_many(f.coords_, to_coords);
      for (auto i = 0u; i < to_coords.size(); ++i) {
        if (to[i].router_id_ != router_id_) {
          continue;
        }
        source_spec s(id_, source_spec::category::BUS,
                      source_spec::type::OSRM_ROUTE);
        from_result.emplace_back(s, router_id_, costs[i]);
        id_++;
      }
      result.push_back(std::move(from_result));
    }
    return result;
  }

  std::vector<node_ref> close_nodes(geo::latlng const& latlng) {
    NearestParameters params;
    params.number_of_results = 3;
    params.coordinates.push_back(make_coord(latlng));

    Object result;
    auto const status = osrm_->Nearest(params, result);
    if (status != Status::Ok) {
      return {};
    }
    std::vector<node_ref> refs;
    for (auto const& w : result.values["waypoints"].get<Array>().values) {
      auto const waypoint_object = w.get<json::Object>();
      auto const waypoint_location =
          waypoint_object.values.at("location").get<json::Array>().values;
      node_ref ref({waypoint_location[1].get<json::Number>().value,
                    waypoint_location[0].get<json::Number>().value},
                   node_ref_id(id_), router_id_);
      refs.emplace_back(ref);
    }
    return refs;
  }

  std::unique_ptr<OSRM> osrm_;
  size_t id_ = 0;
  size_t router_id_;
};

osrm_routing::osrm_routing(std::size_t router_id, std::string path)
    : impl_(std::make_unique<osrm_routing::impl>(router_id, path)) {}
osrm_routing::~osrm_routing() = default;

std::vector<std::vector<routing_result>> osrm_routing::find_routes(
    std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
  return impl_->find_routes(from, to);
}

std::vector<node_ref> osrm_routing::close_nodes(geo::latlng const& latlng) {
  return impl_->close_nodes(latlng);
}

geo::polyline osrm_routing::get_polyline(node_ref const& from,
                                         node_ref const& to) {
  return impl_->get_polyline(from, to);
}

}  // namespace path
}  // namespace motis
