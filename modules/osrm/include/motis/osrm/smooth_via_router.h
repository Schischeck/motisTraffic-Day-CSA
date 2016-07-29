#pragma once

#include <memory>

#include "osrm-backend/data_structures/search_engine.hpp"
#include "osrm-backend/server/data_structures/internal_datafacade.hpp"

#include "motis/module/message.h"

#include "motis/osrm/error.h"

using namespace motis::module;

namespace motis {
namespace osrm {

template <typename DataFacadeT>
struct smooth_via_router {
  using search_engine = SearchEngine<DataFacadeT>;

  struct leg_result {
    int time;
    double distance;
    std::vector<FixedPointCoordinate> polyline;
  };

  struct via_result {
    int time;
    double distance;
    std::vector<std::vector<FixedPointCoordinate>> polylines;
  };

  smooth_via_router(DataFacadeT* data_facade)
      : facade_(data_facade),
        search_engine_(std::make_unique<search_engine>(data_facade)) {}

  motis::module::msg_ptr smooth_via(OSRMSmoothViaRouteRequest const* req) {
    auto resolved_nodes = resolve_nodes(req);
    auto leg_results = route_legs(resolved_nodes);

    auto result = find_best(leg_results, -1, -1);

    return result_to_message(result);
  }

  std::vector<std::vector<PhantomNode>> resolve_nodes(
      OSRMSmoothViaRouteRequest const* req) {
    std::vector<std::vector<PhantomNode>> resolved_nodes;
    for (auto&& waypoint : *req->waypoints()) {
      std::vector<PhantomNode> nodes;
      for (auto&& pos : *waypoint->positions()) {
        auto coord = FixedPointCoordinate{
            static_cast<int>(pos->lat() * COORDINATE_PRECISION),
            static_cast<int>(pos->lng() * COORDINATE_PRECISION)};
        std::vector<std::pair<PhantomNode, double>> result;
        facade_->IncrementalFindPhantomNodeForCoordinateWithMaxDistance(
            coord, result, 25.);

        if (!result.empty()) {
          for (auto const& pair : result) {
            nodes.push_back(pair.first);
          }
        } else {
          facade_->IncrementalFindPhantomNodeForCoordinate(coord, nodes, 2);
        }
      }

      std::sort(begin(nodes), end(nodes), [](auto&& lhs, auto&& rhs) {
        return std::tie(lhs.location.lat, lhs.location.lon) <
               std::tie(rhs.location.lat, rhs.location.lon);
      });
      nodes.erase(std::unique(begin(nodes), end(nodes)), end(nodes));
      resolved_nodes.emplace_back(std::move(nodes));
    }

    if (resolved_nodes.size() < 2) {
      throw std::system_error(error::invalid_request);
    }

    return resolved_nodes;
  }

  std::vector<std::vector<std::vector<leg_result>>> route_legs(
      std::vector<std::vector<PhantomNode>> const& resolved_nodes) {
    std::vector<std::vector<std::vector<leg_result>>> leg_results;
    for (auto i = 1ul; i < resolved_nodes.size(); ++i) {
      std::vector<std::vector<leg_result>> many_to_many_results;
      for (auto const& start_node : resolved_nodes[i - 1]) {
        std::vector<leg_result> one_to_many_results;
        for (auto const& end_node : resolved_nodes[i]) {
          one_to_many_results.push_back(route_direct(start_node, end_node));
        }
        many_to_many_results.push_back(one_to_many_results);
      }
      leg_results.push_back(many_to_many_results);
    }

    return leg_results;
  }

  leg_result route_direct(PhantomNode const& from, PhantomNode const& to) {
    InternalRouteResult raw_route;
    raw_route.segment_end_coordinates.emplace_back(PhantomNodes{from, to});

    search_engine_->direct_shortest_path(
        raw_route.segment_end_coordinates, {false},
        raw_route);  // TODO correct u-turn behavior

    if (INVALID_EDGE_WEIGHT == raw_route.shortest_path_length) {
      std::cout << "Error occurred, single path not found" << std::endl;

      return {std::numeric_limits<int>::max(),
              std::numeric_limits<double>::max(),
              {}};
    }

    leg_result result{0, 0.0, {}};

    if (raw_route.unpacked_path_segments.size() != 1) {
      std::cout << "unpacked path segments have unexpected size"
                << raw_route.unpacked_path_segments.size() << std::endl;
    }

    result.polyline.push_back(
        raw_route.segment_end_coordinates[0].source_phantom.location);
    for (auto const& data : raw_route.unpacked_path_segments[0]) {
      result.polyline.push_back(facade_->GetCoordinateOfNode(data.node));
    }
    result.polyline.push_back(
        raw_route.segment_end_coordinates[0].target_phantom.location);

    for (unsigned i = 1; i < result.polyline.size(); ++i) {
      result.distance += coordinate_calculation::euclidean_distance(
          result.polyline[i - 1], result.polyline[i]);
    }

    result.time = get_time(raw_route.segment_end_coordinates[0].source_phantom,
                           raw_route.source_traversed_in_reverse[0]);
    for (auto const& data : raw_route.unpacked_path_segments[0]) {
      result.time += data.segment_duration;
    }
    result.time += get_time(
        raw_route.segment_end_coordinates[0].target_phantom,
        raw_route.target_traversed_in_reverse[0]);  // TODO intermediate
    // phantoms are counted
    // twice

    result.time /= 10;

    return result;
  }

  int get_time(PhantomNode const& node, bool traversed_in_reverse) {
    return traversed_in_reverse ? node.reverse_weight : node.forward_weight;
  }

  std::map<std::pair<size_t, size_t>, via_result> result_cache;

  via_result find_best_cached(
      std::vector<std::vector<std::vector<leg_result>>> const& leg_results,
      size_t depth, size_t node_idx) {
    auto it = result_cache.find({depth, node_idx});
    if (it != end(result_cache)) {
      return it->second;
    }

    auto result = find_best(leg_results, depth, node_idx);
    result_cache[{depth, node_idx}] = result;
    std::cout << result.time << std::endl;
    return result;
  }

  via_result find_best(
      std::vector<std::vector<std::vector<leg_result>>> const& leg_results,
      size_t depth, size_t node_idx) {
    if (depth == -1 && node_idx == -1) {
      result_cache.clear();
    }

    if (depth == leg_results.size() - 1) {
      auto min = std::min_element(
          begin(leg_results[depth][node_idx]),
          end(leg_results[depth][node_idx]),
          [](auto&& lhs, auto&& rhs) { return lhs.time < rhs.time; });
      return {min->time, min->distance, {min->polyline}};
    }

    std::vector<via_result> tails;
    for (auto i = 0; i < leg_results[depth + 1].size(); ++i) {
      auto tail = find_best_cached(leg_results, depth + 1, i);

      if (tail.time == std::numeric_limits<int>::max()) {
        continue;
      }

      if (depth != -1) {
        auto self = leg_results[depth][node_idx][i];

        if (self.time == std::numeric_limits<int>::max()) {
          continue;
        }

        tail.time += self.time;
        tail.distance += self.distance;
        tail.polylines.insert(begin(tail.polylines), self.polyline);
      }

      tails.push_back(tail);
    }

    if (tails.empty()) {
      return {std::numeric_limits<int>::max(),
              std::numeric_limits<double>::max(),
              {{}}};
    }

    return *std::min_element(
        begin(tails), end(tails),
        [](auto&& lhs, auto&& rhs) { return lhs.time < rhs.time; });
  }


  motis::module::msg_ptr result_to_message(via_result const& result) {
    motis::module::message_creator mc;
    std::vector<flatbuffers::Offset<Polyline>> segments;
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

  DataFacadeT* facade_;
  std::unique_ptr<search_engine> search_engine_;
};

}  // namespace osrm
}  // namespace motis
