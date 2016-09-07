#include "motis/routes/prepare/rel/match_connector.h"

namespace motis {
namespace routes {

void build_graph(station_seq const& seq, std::vector<match_seq>& matches) {

  std::vector<std::unique_ptr<station_p>> nodes;

  create_nodes(nodes, matches);

  create_missing_nodes(seq, nodes);

  create_edges(nodes);

  for (auto& node : nodes) {
    std::cout << "\n"
              << "node " << node->station_ << " : ";
    for (auto const& edge : node->edges_) {
      std::cout << edge.from_->station_ << "|" << edge.to_->station_ << " ";
    }
  }
}

void create_nodes(std::vector<std::unique_ptr<station_p>>& nodes,
                  std::vector<match_seq>& matches) {
  auto offset = 0;
  for (auto i = 0u; i < matches.size(); i++) {
    offset = nodes.size();
    for (auto j = 0u; j < matches[i].stations_.size(); j++) {
      auto station = matches[i].stations_[j];
      nodes.emplace_back(std::make_unique<station_p>(
          station.first, i, matches[i].polyline_[station.second]));
      if (j > 0) {
        auto prev_station = matches[i].stations_[j - 1];
        nodes[offset + j - 1]->edges_.emplace_back(
            nodes[offset + j - 1].get(), nodes[offset + j].get(),
            matches[i].polyline_, prev_station.second,
            station.second - prev_station.second, 0);
      }
    }
  }
}

void create_missing_nodes(station_seq const& seq,
                          std::vector<std::unique_ptr<station_p>>& nodes) {
  std::vector<size_t> missing_nodes;

  for (auto i = 0u; i < seq.station_ids_.size(); i++) {
    auto node = std::find_if(begin(nodes), end(nodes),
                             [&](auto&& n) { return n->station_ == i; });
    if (node != end(nodes)) {
      continue;
    }
    missing_nodes.push_back(i);
  }

  for (auto const& s : missing_nodes) {
    nodes.emplace_back(std::make_unique<station_p>(s, -1, seq.coordinates_[s]));
  }
}

void create_edges(std::vector<std::unique_ptr<station_p>>& nodes) {

  for (auto& n1 : nodes) {
    for (auto& n2 : nodes) {
      if (n1->match_ != n2->match_ && n1->station_ == n2->station_) {
        std::vector<latlng> polyline = find_route(n1->coords_, n2->coords_);
        n1->edges_.emplace_back(n1.get(), n2.get(), polyline, 0, 2,
                                calc_weight(polyline));
      }
    }

    for (auto& n2 : nodes) {
      if (n1->match_ != n2->match_ && (n1->station_ + 1 == n2->station_)) {

        auto edge = std::find_if(
            begin(n1->edges_), end(n1->edges_),
            [&](auto const& e) { return e.to_->station_ == n2->station_; });

        if (edge != end(n1->edges_)) {
          continue;
        }

        std::vector<latlng> polyline = find_route(n1->coords_, n2->coords_);
        n1->edges_.emplace_back(n1.get(), n2.get(), polyline, 0, 2,
                                calc_weight(polyline));
      }
    }
  }
}

float calc_weight(std::vector<latlng> const& polyline) {
  auto weight = 0;
  for (auto i = 0u; i < polyline.size(); i++) {
    if (i > 0) {
      weight += geo_detail::distance_in_m(polyline[i - 1], polyline[i]);
    }
  }
  return weight;
}

std::vector<latlng> find_route(latlng const& p1, latlng const& p2) {
  return {p1, p2};
}

void connect_matches(std::vector<station_seq> const& sequences,
                     std::vector<std::vector<match_seq>>& matches) {

  std::vector<std::vector<station_p>> nodes;

  for (auto i = 0u; i < sequences.size(); i++) {
    build_graph(sequences[i], matches[i]);
  }
}

}  // namespace motis
}  // namespace routes
