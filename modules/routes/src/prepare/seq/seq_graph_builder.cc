#include "motis/routes/prepare/seq/seq_graph_builder.h"

#include "motis/core/common/transform_to_vec.h"

#include "motis/routes/prepare/prepare_data.h"

using namespace geo;

namespace motis {
namespace routes {

void add_matches(seq_graph& g, source_spec::category const& category,
                 std::vector<match_seq> const& matches) {
  auto& nodes = g.nodes_;
  for (auto const& m : matches) {
    if (m.source_.category_ != category) {
      continue;
    }

    for (auto j = 0u; j < m.stations_.size(); ++j) {
      auto const station = m.stations_[j];
      auto const node_idx = g.nodes_.size();

      nodes.emplace_back(std::make_unique<seq_node>(
          node_idx, station.first, node_ref{m.polyline_[station.second]}));
      g.station_to_nodes_[station.first].push_back(nodes.back().get());

      if (j > 0) {
        auto const prev_station = m.stations_[j - 1];
        std::vector<geo::latlng> polyline;
        polyline.insert(begin(polyline),
                        begin(m.polyline_) + prev_station.second,
                        begin(m.polyline_) + station.second + 1);

        nodes[node_idx - 1]->edges_.emplace_back(nodes[node_idx - 1].get(),
                                                 nodes[node_idx].get(),
                                                 polyline, m.source_, 0);
      }
    }
  }
}

node_ref create_ref(station_seq const& seq, std::string const& id) {
  auto it = std::find(begin(seq.station_ids_), end(seq.station_ids_), id);
  node_ref ref;
  if (it == end(seq.station_ids_)) {
    return ref;
  }
  ref.coords_ = seq.coordinates_[std::distance(begin(seq.station_ids_), it)];
  ref.id_ = -1;
  return ref;
}

void add_close_nodes(seq_graph& g, station_seq const& seq,
                     strategies& routing_strategies,
                     source_spec::category const& category) {
  for (auto i = 0u; i < seq.station_ids_.size(); ++i) {
    for (auto const& node :
         get_strategy(routing_strategies, category)
             ->close_nodes(create_ref(seq, seq.station_ids_[i]))) {
      g.nodes_.emplace_back(
          std::make_unique<seq_node>(g.nodes_.size(), i, node));
      g.station_to_nodes_[i].push_back(g.nodes_.back().get());
    }
  }
}

void connect_nodes(std::vector<seq_node*>& from_nodes,
                   std::vector<seq_node*> const& to_nodes,
                   std::vector<std::vector<routing_result>> const& polylines) {
  for (auto i = 0u; i < from_nodes.size(); ++i) {
    for (auto j = 0u; j < to_nodes.size(); ++j) {
      auto& from = from_nodes[i];
      auto const& to = to_nodes[j];

      if (from->idx_ == to->idx_) {
        continue;
      }

      if (std::find_if(begin(from->edges_), end(from->edges_),
                       [&](auto const& e) {
                         return e.to_->idx_ == to->idx_;
                       }) != end(from->edges_)) {
        continue;
      }

      auto const& p = polylines[i][j];
      polyline polyline;
      from->edges_.emplace_back(from, to, polyline, p.source_, p.weight_);
    }
  }
}

void create_edges(seq_graph& g, strategies& routing_strategies,
                  source_spec::category const& category) {
  auto refs = transform_to_vec(g.station_to_nodes_, [](auto const& sn) {
    return transform_to_vec(sn, [](auto const& node) { return node->ref_; });
  });

  for (auto i = 0u; i < g.station_to_nodes_.size() - 1; ++i) {
    if (i != 0) {
      connect_nodes(g.station_to_nodes_[i], g.station_to_nodes_[i],
                    get_strategy(routing_strategies, category)
                        ->find_routes(refs[i], refs[i]));
    }

    connect_nodes(g.station_to_nodes_[i], g.station_to_nodes_[i + 1],
                  get_strategy(routing_strategies, category)
                      ->find_routes(refs[i], refs[i + 1]));
  }
}

seq_graph build_seq_graph(source_spec::category const& category,
                          station_seq const& seq,
                          std::vector<match_seq> const& matches,
                          strategies& routing_strategies) {
  seq_graph g{seq.station_ids_.size()};

  add_matches(g, category, matches);
  add_close_nodes(g, seq, routing_strategies, category);
  create_edges(g, routing_strategies, category);

  g.initials_ = transform_to_vec(g.station_to_nodes_.front(),
                                 [](auto&& node) { return node->idx_; });
  g.goals_ = transform_to_vec(g.station_to_nodes_.back(),
                              [](auto&& node) { return node->idx_; });

  return g;
}

}  // namespace motis
}  // namespace routes
