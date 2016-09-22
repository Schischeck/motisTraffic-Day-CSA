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
      auto coords = matches[i].polyline_[station.second];

      g.nodes_.emplace_back(std::make_unique<station_p>(
          g.nodes_.size(), station.first, i, coords));
      g.station_to_nodes_[station.first].push_back(g.nodes_.back().get());
      g.node_refs_.emplace_back(coords, -1);

      if (j > 0) {
        auto prev_station = matches[i].stations_[j - 1];
        std::vector<geo::latlng> polyline;
        polyline.insert(begin(polyline),
                        begin(matches[i].polyline_) + prev_station.second,
                        begin(matches[i].polyline_) + station.second);

        g.nodes_[offset + j - 1]->edges_.emplace_back(
            g.nodes_[offset + j - 1].get(), g.nodes_[offset + j].get(),
            polyline, 0);
      }
    }
  }
}

void create_additional_nodes(seq_graph& g, station_seq const& seq,
                             routing_strategy& strategy) {

  for (auto i = 0u; i < seq.station_ids_.size(); i++) {
    auto const& close_nodes = strategy.close_nodes(seq.station_ids_[i]);

    for (auto const& node : close_nodes) {
      g.nodes_.emplace_back(
          std::make_unique<station_p>(g.nodes_.size(), i, -1, node.coords_));
      g.station_to_nodes_[i].push_back(g.nodes_.back().get());
      g.node_refs_.push_back(node);
    }
  }
}

void connect_nodes(std::vector<station_p*>& station1,
                   std::vector<station_p*> const& station2,
                   std::vector<std::vector<routing_result>> const& polylines) {
  for (auto i = 0u; i < station1.size(); i++) {
    for (auto j = 0u; j < station2.size(); j++) {
      auto& n1 = station1[i];
      auto const& n2 = station2[j];
      if (n1->match_ != -1 && n2->match_ != -1 && n1->match_ == n2->match_ &&
          n1->station_ == n2->station_) {
        continue;
      }

      auto edge =
          std::find_if(begin(n1->edges_), end(n1->edges_),
                       [&](auto const& e) { return e.to_->idx_ == n2->idx_; });

      if (edge != end(n1->edges_)) {
        continue;
      }
      n1->edges_.emplace_back(n1, n2, polylines[i][j].polyline_,
                              polylines[i][j].weight_);
    }
  }
}

void get_node_refs(size_t const& station_id, seq_graph const& g,
                   std::vector<node_ref>& refs) {
  std::for_each(begin(g.station_to_nodes_[station_id]),
                end(g.station_to_nodes_[station_id]), [&](auto&& n) {
                  refs.push_back(g.node_refs_[n->idx_ > 0 ? n->idx_ - 1 : 0]);
                });
}

void create_edges(seq_graph& g, routing_strategy& strategy) {
  for (auto i = 0u; i < g.station_to_nodes_.size() - 1; i++) {

    std::vector<node_ref> refs1;
    get_node_refs(i, g, refs1);

    if (i != 0) {
      auto const& polylines = strategy.find_routes(refs1, refs1);
      connect_nodes(g.station_to_nodes_[i], g.station_to_nodes_[i], polylines);
    }

    std::vector<node_ref> refs2;
    get_node_refs(i + 1, g, refs2);

    auto const& polylines = strategy.find_routes(refs1, refs2);
    connect_nodes(g.station_to_nodes_[i], g.station_to_nodes_[i + 1],
                  polylines);
  }
}

void add_inits_goals(seq_graph& g) {
  for (auto const& seq : g.station_to_nodes_.front()) {
    g.initials_.push_back(seq->idx_);
  }
  for (auto const& seq : g.station_to_nodes_.back()) {
    g.goals_.push_back(seq->idx_);
  }
}

seq_graph build_seq_graph(station_seq const& seq,
                          std::vector<match_seq> const& matches) {
  seq_graph g;
  g.station_to_nodes_.resize(seq.station_ids_.size());
  stub_routing strategy(seq);

  create_nodes(g, matches);
  create_additional_nodes(g, seq, strategy);
  create_edges(g, strategy);
  add_inits_goals(g);
  return g;
}

}  // namespace motis
}  // namespace routes
