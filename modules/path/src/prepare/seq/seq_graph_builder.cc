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

struct station_cluster {
  explicit station_cluster(std::string station_id)
      : station_id_(std::move(station_id)) {}

  std::string station_id_;
  std::vector<std::pair<node_ref, seq_node*>> nodes_;
};

void insert_edges(station_cluster& from, station_cluster const& to,
                  routing_result_matrix const& results) {
  results.verify_dimensions(from.nodes_.size(), to.nodes_.size());

  for (auto i = 0u; i < from.nodes_.size(); ++i) {
    for (auto j = 0u; j < to.nodes_.size(); ++j) {
      auto& from_node = from.nodes_[i].second;
      auto const& to_node = to.nodes_[j].second;

      if (from_node->ref_ == to_node->ref_) {
        continue;
      }

      auto result = results.get(i, j);
      if (!result.is_valid()) {
        continue;
      }

      from_node->edges_.emplace_back(from_node, to_node, result);
      ++to_node->incomming_edges_count_;
    }
  }
}

routing_result_matrix route(station_cluster const& from,
                            station_cluster const& to,
                            routing_strategy const* s, distance_cache& cache) {
  if (!s->is_cacheable()) {
    auto const to_ref = [](auto const& pair) { return pair.first; };
    return s->find_routes(utl::to_vec(from.nodes_, to_ref),
                          utl::to_vec(to.nodes_, to_ref));
  }

  distance_cache::key key{from.station_id_, to.station_id_, s->strategy_id()};

  auto cached_result = cache.get(key);
  if (cached_result.is_valid()) {

    return cached_result;
  } else {
    auto const to_ref = [](auto const& pair) { return pair.first; };
    auto routed_result = s->find_routes(utl::to_vec(from.nodes_, to_ref),
                                        utl::to_vec(to.nodes_, to_ref));
    cache.put(key, routed_result);
    return routed_result;
  }
}

void create_edges(seq_graph& graph, station_seq const& seq,
                  routing_strategy const* s, distance_cache& cache) {
  auto clusters = utl::to_vec(
      seq.station_ids_, [](auto const& id) { return station_cluster{id}; });

  for (auto& node : graph.nodes_) {
    if (!s->can_route(node->ref_)) {
      continue;
    }

    clusters[node->station_idx_].nodes_.emplace_back(node->ref_, node.get());
  }

  for (auto& cluster : clusters) {
    std::sort(begin(cluster.nodes_), end(cluster.nodes_));
  }

  for (auto i = 0u; i < graph.seq_size_ - 1; ++i) {  // between stations
    if (clusters[i].nodes_.empty() || clusters[i + 1].nodes_.empty()) {
      continue;
    }

    insert_edges(clusters[i], clusters[i + 1],
                 route(clusters[i], clusters[i + 1], s, cache));
  }
  for (auto i = 1u; i < graph.seq_size_ - 1; ++i) {  // within station
    if (clusters[i].nodes_.empty()) {
      continue;
    }

    insert_edges(clusters[i], clusters[i],
                 route(clusters[i], clusters[i], s, cache));
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
std::map<strategy_id_t, std::vector<size_t>> added_close_nodes;

seq_graph build_seq_graph(station_seq const& seq,
                          std::vector<routing_strategy*> const& strategies,
                          distance_cache& cache) {
  namespace sc = std::chrono;

  seq_graph graph{seq.station_ids_.size()};
  for (auto const& strategy : strategies) {
    auto const& n_before = graph.nodes_.size();
    auto const t_0 = sc::steady_clock::now();
    add_close_nodes(graph, seq, strategy);
    auto const t_1 = sc::steady_clock::now();
    create_edges(graph, seq, strategy, cache);
    auto const t_2 = sc::steady_clock::now();

    utl::erase_if(graph.nodes_, [](auto const& node) {
      return node->edges_.empty() && node->incomming_edges_count_ == 0;
    });
    auto const& n_after = graph.nodes_.size();

    // std::cout << "\n
    // =====================================================\n";
    // print_seq_graph(graph);

    std::lock_guard<std::mutex> lock(perf_stats_mutex);
    create_nodes_timings[strategy->strategy_id()].push_back(
        sc::duration_cast<sc::milliseconds>(t_1 - t_0).count());
    create_edges_timings[strategy->strategy_id()].push_back(
        sc::duration_cast<sc::milliseconds>(t_2 - t_1).count());
    added_close_nodes[strategy->strategy_id()].push_back(
        (n_after - n_before) / seq.station_ids_.size());
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

  std::cout << " === added close nodes ===" << std::endl;
  dump(added_close_nodes);
}

}  // namespace path
}  // namespace motis
