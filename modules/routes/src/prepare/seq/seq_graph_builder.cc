#include "motis/routes/prepare/seq/seq_graph_builder.h"

using namespace motis::geo;

namespace motis {
namespace routes {

void create_nodes(seq_graph& g, std::vector<match_seq> const& matches) {
  auto offset = 0;
  for (auto i = 0u; i < matches.size(); i++) {
    offset = g.nodes_.size();
    for (auto j = 0u; j < matches[i].stations_.size(); j++) {

      auto station = matches[i].stations_[j];

      g.nodes_.emplace_back(
          std::make_unique<station_p>(g.nodes_.size(), station.first, i,
                                      matches[i].polyline_[station.second]));
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

void create_unmatched_nodes(seq_graph& g, station_seq const& seq) {
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
    g.nodes_.emplace_back(std::make_unique<station_p>(g.nodes_.size(), s, -1,
                                                      seq.coordinates_[s]));
    g.station_to_nodes_[s].push_back(g.nodes_.back().get());
  }
}

std::vector<latlng> find_route(latlng const& p1, latlng const& p2) {
  return {p1, p2};
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
      n1->edges_.emplace_back(n1, n2, polyline, 0, 2, length(polyline));
    }
  }
}

void create_edges(seq_graph& g) {
  for (auto i = 0u; i < g.station_to_nodes_.size(); i++) {
    connect_nodes(g.station_to_nodes_[i], g.station_to_nodes_[i]);
    if (i < g.station_to_nodes_.size() - 1) {
      connect_nodes(g.station_to_nodes_[i], g.station_to_nodes_[i + 1]);
    }
  }
}

seq_graph build_seq_graph(station_seq const& seq,
                          std::vector<match_seq> const& matches) {
  seq_graph g;
  g.station_to_nodes_.resize(seq.station_ids_.size());

  create_nodes(g, matches);
  create_unmatched_nodes(g, seq);
  create_edges(g);

  return g;
}

}  // namespace motis
}  // namespace routes
