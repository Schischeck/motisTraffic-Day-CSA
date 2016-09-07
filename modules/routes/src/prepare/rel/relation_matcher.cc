#include "motis/routes/prepare/rel/relation_matcher.h"

#include "motis/core/common/logging.h"
#include "motis/routes/prepare/point_rtree.h"

namespace motis {
namespace routes {

std::vector<std::vector<match_seq>> match_sequences(
    std::vector<std::vector<latlng>> const& polylines,
    std::vector<station_seq> const& sequences,
    std::map<std::string, std::vector<latlng>> const& bus_stops) {
  std::vector<std::vector<match_seq>> result(sequences.size(),
                                             std::vector<match_seq>());

  for (auto const& polyline : polylines) {
    auto rtree = make_point_rtree(polyline, [&](auto&& c) {
      return point_rtree::point{c.lng_, c.lat_};
    });

    for (auto const& seq : sequences) {
      long last_node = -1;
      long seq_count = 0;
      long start_node = -1;
      std::vector<std::pair<size_t, size_t>> stations;
      for (auto i = 0u; i < seq.coordinates_.size(); ++i) {
        auto close_nodes = rtree.in_radius(seq.coordinates_[i].lat_,
                                           seq.coordinates_[i].lng_, 100);

        auto bus_nodes = bus_stops.find(seq.station_ids_[i]);

        if (bus_nodes != end(bus_stops)) {
          std::for_each(begin(bus_nodes->second), end(bus_nodes->second),
                        [&](auto&& stop) {
                          auto close_bus_nodes =
                              rtree.in_radius(stop.lat_, stop.lng_, 100);
                          close_nodes.insert(begin(close_nodes),
                                             begin(close_bus_nodes),
                                             end(close_bus_nodes));
                        });
        }
        if (close_nodes.empty()) {
          if (seq_count >= 2) {
            match_seq match;
            match.stations_ = stations;
            match.polyline_.insert(begin(match.polyline_),
                                   begin(polyline) + start_node,
                                   begin(polyline) + last_node);
            result[i].emplace_back(std::move(match));
          }
          seq_count = 0;
          start_node = -1;
          continue;
        }
        long node = *std::min_element(begin(close_nodes), end(close_nodes));

        if (node <= last_node) {
          break;
        }

        if (start_node == -1) {
          start_node = node;
        }
        stations.emplace_back(i, node);
        last_node = node;
        seq_count++;
        if (i == seq.coordinates_.size() - 1 && seq_count == i) {
          match_seq match;
          match.full_match_ = true;
          match.polyline_ = polyline;
          match.stations_ = stations;
          result[i].emplace_back(std::move(match));
        }
      }
    }
  }
  return result;
}

}  // namespace motis
}  // namespace routes
