#include "motis/path/prepare/seq/seq_graph_builder.h"

#include "utl/to_vec.h"

#include "parser/util.h"

#include "motis/path/prepare/seq/seq_graph_printer.h"

using namespace geo;

namespace motis {
namespace path {

void add_close_nodes(seq_graph& g, station_seq const& seq,
                     routing_strategy* routing_strategy) {
  for (auto i = 0u; i < seq.coordinates_.size(); ++i) {
    for (auto const& node :
         routing_strategy->close_nodes(seq.coordinates_[i])) {
      g.nodes_.emplace_back(
          std::make_unique<seq_node>(g.nodes_.size(), i, node));
      g.station_to_nodes_[i].push_back(g.nodes_.back().get());
    }
  }
}

void connect_nodes(std::vector<seq_node*>& from_nodes,
                   std::vector<seq_node*> const& to_nodes,
                   std::vector<std::vector<routing_result>> const& results) {
  verify(results.size() == from_nodes.size(), "routing 'from' size mismatch");
  for (auto i = 0u; i < from_nodes.size(); ++i) {
    verify(results[i].size() == to_nodes.size(), "routing 'to' size mismatch");
    for (auto j = 0u; j < to_nodes.size(); ++j) {
      auto& from = from_nodes[i];
      auto const& to = to_nodes[j];

      if (from->idx_ == to->idx_) {
        continue;
      }

      auto result = results[i][j];
      if (!result.valid_) {
        continue;
      }

      from->edges_.emplace_back(from, to, result);
    }
  }
}

void create_edges(seq_graph& g, routing_strategy* routing_strategy) {
  auto filtered_nodes = utl::to_vec(g.station_to_nodes_, [&](auto const& sn) {
    std::vector<seq_node*> filtered;
    for (auto const& node : sn) {
      if (routing_strategy->can_route(node->ref_)) {
        filtered.emplace_back(node);
      }
    }
    return filtered;
  });

  auto const refs = utl::to_vec(filtered_nodes, [](auto const& sn) {
    return utl::to_vec(sn, [](auto const& n) { return n->ref_; });
  });

  for (auto i = 0u; i < filtered_nodes.size() - 1; ++i) {
    if (i != 0) {
      connect_nodes(filtered_nodes[i], filtered_nodes[i],
                    routing_strategy->find_routes(refs[i], refs[i]));
    }

    connect_nodes(filtered_nodes[i], filtered_nodes[i + 1],
                  routing_strategy->find_routes(refs[i], refs[i + 1]));
  }
}

seq_graph build_seq_graph(
    station_seq const& seq,
    std::vector<routing_strategy*> const& routing_strategies) {
  seq_graph g{seq.station_ids_.size()};

  for (auto const& s : routing_strategies) {
    add_close_nodes(g, seq, s);
    create_edges(g, s);

    // std::cout << " =====================================================\n ";
    // print_seq_graph(g);
  }

  g.initials_ = utl::to_vec(g.station_to_nodes_.front(),
                            [](auto&& node) { return node->idx_; });
  g.goals_ = utl::to_vec(g.station_to_nodes_.back(),
                         [](auto&& node) { return node->idx_; });
  return g;
}

}  // namespace motis
}  // namespace path
