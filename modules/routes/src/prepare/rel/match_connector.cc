#include "motis/routes/prepare/rel/match_connector.h"

namespace motis {
namespace routes {

void build_graph(station_seq const& seq, std::vector<match_seq>& matches) {

  graph g;

  g.station_to_nodes_.resize(seq.station_ids_.size());

  create_nodes(g, matches);

  create_unmatched_nodes(g, seq);

  create_edges(g);

  std::sort(begin(g.nodes_), end(g.nodes_),
            [](auto&& l, auto&& r) { return l->station_ < r->station_; });

  for (auto& node : g.nodes_) {
    std::cout << "\n"
              << "station " << node->station_ << " : ";
    for (auto const& edge : node->edges_) {
      std::cout << edge.from_->station_ << "|" << edge.to_->station_ << " ";
    }
  }
}

void create_nodes(graph& g, std::vector<match_seq>& matches) {
  auto offset = 0;
  for (auto i = 0u; i < matches.size(); i++) {
    offset = g.nodes_.size();
    for (auto j = 0u; j < matches[i].stations_.size(); j++) {

      auto station = matches[i].stations_[j];

      g.nodes_.emplace_back(std::make_unique<station_p>(
          station.first, i, matches[i].polyline_[station.second]));
      g.station_to_nodes_[station.first].push_back(g.nodes_.back().get());

      if (j > 0) {
        auto prev_station = matches[i].stations_[j - 1];

        g.nodes_[offset + j - 1]->edges_.emplace_back(
            g.nodes_[offset + j - 1].get(), g.nodes_[offset + j].get(),
            matches[i].polyline_, prev_station.second,
            station.second - prev_station.second, 0);
      }
    }
  }
}

void create_unmatched_nodes(graph& g, station_seq const& seq) {
  std::vector<size_t> missing_nodes;

  for (auto i = 0u; i < seq.station_ids_.size(); i++) {
    auto node = std::find_if(begin(g.nodes_), end(g.nodes_),
                             [&](auto&& n) { return n->station_ == i; });
    if (node != end(g.nodes_)) {
      continue;
    }
    missing_nodes.push_back(i);
  }

  for (auto const& s : missing_nodes) {
    g.nodes_.emplace_back(
        std::make_unique<station_p>(s, -1, seq.coordinates_[s]));
    g.station_to_nodes_[s].push_back(g.nodes_.back().get());
  }
}

void create_edges(graph& g) {
  for (auto i = 0u; i < g.station_to_nodes_.size(); i++) {
    connect_nodes(g.station_to_nodes_[i], g.station_to_nodes_[i]);
    if (i < g.station_to_nodes_.size() - 1) {
      connect_nodes(g.station_to_nodes_[i], g.station_to_nodes_[i + 1]);
    }
  }
}

void connect_nodes(std::vector<station_p*>& station1,
                   std::vector<station_p*>& station2) {
  for (auto n1 : station1) {
    for (auto n2 : station2) {
      if (n1->match_ == n2->match_ && n1->station_ == n2->station_) {
        continue;
      }
      auto edge = std::find_if(
          begin(n1->edges_), end(n1->edges_),
          [&](auto const& e) { return e.to_->station_ == n2->station_; });

      if (edge != end(n1->edges_)) {
        continue;
      }

      std::vector<latlng> polyline = find_route(n1->coords_, n2->coords_);
      n1->edges_.emplace_back(n1, n2, polyline, 0, 2, calc_weight(polyline));
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
  for (auto i = 0u; i < sequences.size(); i++) {
    if (matches[i].empty()) {
      continue;
    }
    std::cout << "\n"
              << "Sequence " << i;
    build_graph(sequences[i], matches[i]);
  }
}

}  // namespace motis
}  // namespace routes
