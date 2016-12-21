#include "motis/path/prepare/routing/osrm_strategy.h"

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

#include "parser/util.h"

#include "utl/repeat_n.h"
#include "utl/to_vec.h"

using namespace osrm;
using namespace osrm::util;
using namespace osrm::util::json;

namespace motis {
namespace path {

struct osrm_strategy::impl {
  impl(strategy_id_t const strategy_id, std::string const& path)
      : strategy_id_(strategy_id) {
    EngineConfig config;
    config.storage_config = {path};
    config.use_shared_memory = false;

    osrm_ = std::make_unique<OSRM>(config);
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

    return utl::to_vec(
        result.values["waypoints"].get<Array>().values,
        [&](Value const& wp) -> node_ref {
          auto const& obj = wp.get<Object>();
          auto const& loc = obj.values.at("location").get<Array>().values;
          return {{loc[1].get<Number>().value, loc[0].get<Number>().value},
                  node_ref_id{0},  // TODO
                  strategy_id_};
        });
  }

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
    std::vector<geo::latlng> to_coords;
    std::vector<size_t> to_mask;
    for (auto i = 0u; i < to.size(); ++i) {
      if (to[i].strategy_id() != strategy_id_) {
        continue;
      }
      to_coords.push_back(to[i].coords_);
      to_mask.push_back(i);
    }

    return utl::to_vec(from, [&](auto const& f) -> std::vector<routing_result> {
      if (f.strategy_id() != strategy_id_ || to_coords.empty()) {
        return utl::repeat_n(routing_result{}, to.size());
      }

      auto const costs = one_to_many(f.coords_, to_coords);

      std::vector<routing_result> results;
      for (auto i = 0u; i < to_mask.size(); ++i) {
        while (to_mask[i] != 0 && results.size() < to_mask[i]) {
          results.emplace_back();
        }

        source_spec s(0, source_spec::category::BUS,
                      source_spec::type::OSRM_ROUTE);
        results.emplace_back(s, strategy_id_, costs[i]);
      }

      while (results.size() < to.size()) {
        results.emplace_back();
      }

      return results;
    });
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

  geo::polyline get_polyline(node_ref const& from, node_ref const& to) const {
    verify(from.strategy_id_ == strategy_id_, "osrm bad 'from' strategy_id");
    verify(to.strategy_id_ == strategy_id_, "osrm bad 'to' strategy_id");

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
    auto const points =
        utl::to_vec(get(route, "geometry").get<Array>().values,
                    [](auto&& jc) { return jc.template get<Number>().value; });
    std::vector<geo::latlng> polyline;
    for (auto i = 0u; i < points.size() - 1; i = i + 2) {
      polyline.emplace_back(points[i], points[i + 1]);
    }
    return polyline;
  }

  FloatCoordinate make_coord(geo::latlng const& l) const {
    return FloatCoordinate{FloatLongitude{l.lng_}, FloatLatitude{l.lat_}};
  }

  std::unique_ptr<OSRM> osrm_;
  strategy_id_t strategy_id_;
};

osrm_strategy::osrm_strategy(strategy_id_t strategy_id, std::string const& path)
    : routing_strategy(strategy_id),
      impl_(std::make_unique<osrm_strategy::impl>(strategy_id, path)) {}
osrm_strategy::~osrm_strategy() = default;

std::vector<std::vector<routing_result>> osrm_strategy::find_routes(
    std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
  return impl_->find_routes(from, to);
}

std::vector<node_ref> osrm_strategy::close_nodes(geo::latlng const& latlng) {
  return impl_->close_nodes(latlng);
}

geo::polyline osrm_strategy::get_polyline(node_ref const& from,
                                          node_ref const& to) const {
  return impl_->get_polyline(from, to);
}

}  // namespace path
}  // namespace motis
