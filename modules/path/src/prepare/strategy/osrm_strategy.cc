#include "motis/path/prepare/strategy/osrm_strategy.h"

#include <atomic>

#include "osrm/route_parameters.hpp"

#include "engine/datafacade/internal_datafacade.hpp"  // osrm
#include "engine/plugins/viaroute.hpp"  // osrm
#include "engine/routing_algorithms/multi_target.hpp"  // osrm

#include "util/coordinate.hpp"  // osrm
#include "util/json_container.hpp"  // osrm
#include "util/json_util.hpp"  // osrm

#include "geo/latlng.h"

#include "parser/util.h"

#include "utl/concat.h"
#include "utl/repeat_n.h"
#include "utl/to_vec.h"

#include "motis/core/common/hash_map.h"

using namespace osrm;
using namespace osrm::engine;
using namespace osrm::engine::datafacade;
using namespace osrm::engine::plugins;
using namespace osrm::engine::routing_algorithms;
using namespace osrm::storage;
using namespace osrm::util;
using namespace osrm::util::json;

namespace motis {
namespace path {

FloatCoordinate make_coord(geo::latlng const& pos) {
  return FloatCoordinate{FloatLongitude{pos.lng_}, FloatLatitude{pos.lat_}};
}

geo::latlng make_latlng(FloatCoordinate const& coord) {
  return {static_cast<double>(coord.lat), static_cast<double>(coord.lon)};
}

struct osrm_strategy::impl {
  impl(strategy_id_t const strategy_id, std::vector<station> const& stations,
       std::string const& path)
      : strategy_id_(strategy_id),
        osrm_data_facade_(
            std::make_unique<InternalDataFacade>(StorageConfig{path})),
        osrm_heaps_(std::make_unique<SearchEngineData>()),
        mt_forward_(osrm_data_facade_.get(), *osrm_heaps_),
        mt_backward_(osrm_data_facade_.get(), *osrm_heaps_),
        via_route_(std::make_unique<ViaRoutePlugin>(*osrm_data_facade_, -1)) {
    stations_to_nodes_.set_empty_key("");
    for (auto const& station : stations) {
      auto const nodes_with_dists =
          osrm_data_facade_->NearestPhantomNodesFromBigComponent(
              make_coord(station.pos_), 3);

      stations_to_nodes_[station.id_] =
          utl::to_vec(nodes_with_dists, [&](auto const& node_with_dist) {
            node_mem_.emplace_back(node_with_dist.phantom_node);
            return node_mem_.size() - 1;
          });
    }
  }

  std::vector<node_ref> close_nodes(std::string const& station_id) const {
    auto const it = stations_to_nodes_.find(station_id);
    verify(it != end(stations_to_nodes_), "osrm: unknown station!");

    return utl::to_vec(it->second, [&](auto const& idx) -> node_ref {
      return {strategy_id_, idx, make_latlng(node_mem_[idx].location)};
    });
  }

  bool can_route(node_ref const& ref) const {
    return ref.strategy_id() == strategy_id_;
  }

  routing_result_matrix find_routes(std::vector<node_ref> const& from,
                                    std::vector<node_ref> const& to) {
    if (from.empty() || to.empty()) {
      return routing_result_matrix{};
    }

    auto const pair_to_result = [&](auto const& pair) {
      if (pair.first == INVALID_EDGE_WEIGHT) {
        return routing_result{};
      }

      source_spec s{0, source_spec::category::BUS,
                    source_spec::type::OSRM_ROUTE};
      return routing_result{strategy_id_, s, pair.second};
    };

    auto const route = [&](auto const& from_nodes, auto const& to_nodes,
                           bool forward) {
      std::vector<PhantomNode> query_phantoms{PhantomNode{}};
      utl::concat(query_phantoms, to_nodes);

      return utl::to_vec(from_nodes, [&](auto const& f) {
        query_phantoms[0] = f;

        auto const results = forward ? mt_forward_(query_phantoms)
                                     : mt_backward_(query_phantoms);
        verify(results, "osrm routing error!");

        return utl::to_vec(*results, pair_to_result);
      });
    };

    auto const from_nodes =
        utl::to_vec(from, [&](auto const& f) { return node_mem_[f.id_]; });
    auto const to_nodes =
        utl::to_vec(to, [&](auto const& t) { return node_mem_[t.id_]; });

    if (from_nodes.size() <= to_nodes.size()) {
      return routing_result_matrix{route(from_nodes, to_nodes, true)};
    } else {
      return routing_result_matrix{route(to_nodes, from_nodes, false), true};
    }
  }

  geo::polyline get_polyline(node_ref const& from, node_ref const& to) const {
    verify(from.strategy_id_ == strategy_id_, "osrm bad 'from' strategy_id");
    verify(to.strategy_id_ == strategy_id_, "osrm bad 'to' strategy_id");

    RouteParameters params;
    params.geometries = RouteParameters::GeometriesType::CoordVec1D;
    params.overview = RouteParameters::OverviewType::Full;

    params.coordinates.push_back(node_mem_[from.id_].location);
    params.coordinates.push_back(node_mem_[to.id_].location);

    Object result;
    auto const status = via_route_->HandleRequest(params, result);

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

  strategy_id_t strategy_id_;

  std::unique_ptr<InternalDataFacade> osrm_data_facade_;

  std::unique_ptr<SearchEngineData> osrm_heaps_;
  MultiTargetRouting<BaseDataFacade, true> mt_forward_;
  MultiTargetRouting<BaseDataFacade, false> mt_backward_;

  std::unique_ptr<ViaRoutePlugin> via_route_;

  std::vector<PhantomNode> node_mem_;
  hash_map<std::string, std::vector<size_t>> stations_to_nodes_;
};

osrm_strategy::osrm_strategy(strategy_id_t strategy_id,
                             std::vector<station> const& stations,
                             std::string const& osrm_path)
    : routing_strategy(strategy_id),
      impl_(std::make_unique<osrm_strategy::impl>(strategy_id, stations,
                                                  osrm_path)) {}
osrm_strategy::~osrm_strategy() = default;

std::vector<node_ref> osrm_strategy::close_nodes(
    std::string const& station_id) const {
  return impl_->close_nodes(station_id);
}

bool osrm_strategy::can_route(node_ref const& ref) const {
  return impl_->can_route(ref);
}

routing_result_matrix osrm_strategy::find_routes(
    std::vector<node_ref> const& from, std::vector<node_ref> const& to) const {
  return impl_->find_routes(from, to);
}

geo::polyline osrm_strategy::get_polyline(node_ref const& from,
                                          node_ref const& to) const {
  return impl_->get_polyline(from, to);
}

}  // namespace path
}  // namespace motis
