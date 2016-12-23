#include "motis/path/prepare/strategy/osrm_strategy.h"

#include <atomic>

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
      : strategy_id_(strategy_id), ref_counter_{0} {
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
                  node_ref_id{++ref_counter_},  // TODO
                  strategy_id_};
        });
  }

  bool can_route(node_ref const& ref) const {
    return ref.strategy_id() == strategy_id_;
  }

  std::vector<std::vector<routing_result>> find_routes(
      std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
    auto const to_coords = utl::to_vec(to, [](auto&& t) { return t.coords_; });

    return utl::to_vec(from, [&](auto const& f) {
      return utl::to_vec(one_to_many(f.coords_, to_coords),
                         [&](auto const& cost) {
                           source_spec s{0, source_spec::category::BUS,
                                         source_spec::type::OSRM_ROUTE};
                           return routing_result{s, strategy_id_, cost};
                         });
    });
  }

  std::vector<double> one_to_many(geo::latlng const& one,
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

    std::vector<double> costs;
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
  std::atomic<std::uint32_t> ref_counter_;  // TODO
};

osrm_strategy::osrm_strategy(strategy_id_t strategy_id, std::string const& path)
    : routing_strategy(strategy_id),
      impl_(std::make_unique<osrm_strategy::impl>(strategy_id, path)) {}
osrm_strategy::~osrm_strategy() = default;

std::vector<node_ref> osrm_strategy::close_nodes(geo::latlng const& latlng) {
  return impl_->close_nodes(latlng);
}

bool osrm_strategy::can_route(node_ref const& ref) const {
  return impl_->can_route(ref);
}

std::vector<std::vector<routing_result>> osrm_strategy::find_routes(
    std::vector<node_ref> const& from, std::vector<node_ref> const& to) {
  return impl_->find_routes(from, to);
}

geo::polyline osrm_strategy::get_polyline(node_ref const& from,
                                          node_ref const& to) const {
  return impl_->get_polyline(from, to);
}

}  // namespace path
}  // namespace motis
