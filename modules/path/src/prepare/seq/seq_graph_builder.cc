#include "motis/path/prepare/seq/seq_graph_builder.h"

#include <chrono>
#include <iomanip>
#include <mutex>
#include <numeric>

#include "utl/erase_if.h"
#include "utl/repeat_n.h"
#include "utl/to_vec.h"

#include "parser/util.h"

#include "motis/path/prepare/seq/seq_graph_printer.h"

using namespace geo;

namespace motis {
namespace path {

size_t temp_idx = 0;  // no need for synchronization; only used for debug output
void add_close_nodes(seq_graph& g, station_seq const& s,
                     routing_strategy* routing_strategy) {
  for (auto i = 0u; i < s.coordinates_.size(); ++i) {
    for (auto const& node : routing_strategy->close_nodes(s.station_ids_[i])) {
      g.nodes_.emplace_back(std::make_unique<seq_node>(temp_idx++, i, node));
    }
  }
}

void insert_edges(std::vector<seq_node*>& from_nodes,
                  std::vector<seq_node*> const& to_nodes,
                  std::vector<std::vector<routing_result>> const& results) {
  verify(results.size() == from_nodes.size(), "routing 'from' size mismatch");
  for (auto i = 0u; i < from_nodes.size(); ++i) {
    verify(results[i].size() == to_nodes.size(), "routing 'to' size mismatch");
    for (auto j = 0u; j < to_nodes.size(); ++j) {
      auto& from = from_nodes[i];
      auto const& to = to_nodes[j];

      if (from->ref_ == to->ref_) {
        continue;
      }

      auto result = results[i][j];
      if (!result.is_valid()) {
        continue;
      }

      from->edges_.emplace_back(from, to, result);
      ++to->incomming_edges_count_;
    }
  }
}

void create_edges(seq_graph& graph, routing_strategy* s) {
  auto station_vecs = utl::repeat_n(
      std::vector<std::pair<node_ref, seq_node*>>{}, graph.seq_size_);
  for (auto& node : graph.nodes_) {
    if (!s->can_route(node->ref_)) {
      continue;
    }

    station_vecs.emplace_back(node->ref_, node.get());
  }

  for (auto& station_vec : station_vecs) {
    std::sort(begin(station_vec), end(station_vec));
  }

  auto const nodes = utl::to_vec(station_vecs, [](auto const& station_vec) {
    return utl::to_vec(station_vec,
                       [](auto const& pair) { return pair.second; });
  });

  auto const refs = utl::to_vec(station_vecs, [](auto const& station_vec) {
    return utl::to_vec(station_vec,
                       [](auto const& pair) { return pair.second; });
  });

  // TODO

  for (auto i = 0u; i < graph.seq_size_ - 1; ++i) {
    insert_edges(nodes[i], nodes[i + 1], s->find_routes(refs[i], refs[i + 1]));
  }
  for (auto i = 1u; i < graph.seq_size_ - 1; ++i) {
    insert_edges(nodes[i], nodes[i], s->find_routes(refs[i], refs[i]));
  }
}

void finish_graph(seq_graph& graph) {
  size_t idx = 0;
  for (auto const& node : graph.nodes_) {
    node->idx_ = idx++;

    if (node->station_idx_ == 0) {
      graph.initials_.emplace_back(node->idx_);
    } else if (node->station_idx_ == (graph.seq_size_ - 1)) {
      graph.goals_.emplace_back(node->idx_);
    }
  }
}

std::mutex perf_stats_mutex;
std::map<strategy_id_t, std::vector<size_t>> create_nodes_timings;
std::map<strategy_id_t, std::vector<size_t>> create_edges_timings;

seq_graph build_seq_graph(station_seq const& seq,
                          std::vector<routing_strategy*> const& strategies) {
  namespace sc = std::chrono;

  seq_graph graph{seq.station_ids_.size()};
  for (auto const& strategy : strategies) {
    auto const t_0 = sc::steady_clock::now();
    add_close_nodes(graph, seq, strategy);
    auto const t_1 = sc::steady_clock::now();
    create_edges(graph, strategy);
    auto const t_2 = sc::steady_clock::now();

    utl::erase_if(graph.nodes_, [](auto const& node) {
      return node->edges_.empty() && node->incomming_edges_count_ == 0;
    });

    // std::cout << "\n
    // =====================================================\n";
    // print_seq_graph(graph);

    std::lock_guard<std::mutex> lock(perf_stats_mutex);
    create_nodes_timings[strategy->strategy_id()].push_back(
        sc::duration_cast<sc::milliseconds>(t_1 - t_0).count());
    create_edges_timings[strategy->strategy_id()].push_back(
        sc::duration_cast<sc::milliseconds>(t_2 - t_1).count());
  }

  finish_graph(graph);
  return graph;
}

void dump_build_seq_graph_timings() {
  std::lock_guard<std::mutex> lock(perf_stats_mutex);

  auto const dump = [](auto& timings) {
    for (auto& pair : timings) {
      auto& vec = pair.second;
      auto const avg = std::accumulate(begin(vec), end(vec), 0) / vec.size();

      std::sort(begin(vec), end(vec));
      std::cout << pair.first << " | "  //
                << " count:" << std::setw(8) << vec.size()  //
                << " avg:" << std::setw(8) << avg  //
                << " q75:" << std::setw(8) << vec[0.75 * (vec.size() - 1)]  //
                << " q90:" << std::setw(8) << vec[0.90 * (vec.size() - 1)]  //
                << " q95:" << std::setw(8) << vec[0.95 * (vec.size() - 1)]  //
                << std::endl;
    }

  };

  std::cout << " === create nodes  timings === " << std::endl;
  dump(create_nodes_timings);

  std::cout << " === create edges  timings === " << std::endl;
  dump(create_edges_timings);
}

}  // namespace path
}  // namespace motis
