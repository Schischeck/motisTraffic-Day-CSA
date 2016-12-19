#include "motis/path/prepare/prepare_data.h"

#include "parser/util.h"

#include "motis/core/common/logging.h"

#include "motis/path/db/db_builder.h"
#include "motis/path/db/kv_database.h"
#include "motis/path/prepare/parallel_for.h"
#include "motis/path/prepare/rail/geojson.h"
#include "motis/path/prepare/rel/relation_matcher.h"
#include "motis/path/prepare/seq/seq_graph_builder.h"
#include "motis/path/prepare/seq/seq_graph_dijkstra.h"
#include "motis/path/prepare/station_sequences.h"
#include "motis/path/prepare/vector_utils.h"

using namespace motis;
using namespace motis::path;

namespace motis {
namespace path {

void strategy_prepare(std::vector<station_seq> const& sequences,
                      strategies& routing_strategies, kv_database& db) {
  motis::logging::manual_timer timer("preparing data");

  db_builder builder(db);
  std::vector<station_seq> s;
  for (auto const seq : sequences) {
    if (std::find(begin(seq.station_ids_), end(seq.station_ids_), "0104736") !=
        end(seq.station_ids_)) {
      s.push_back(seq);
      break;
    }
  }
  parallel_for("searching routes", sequences, 250, [&](auto const& seq) {

    for (auto const& category_group : category_groups(seq.categories_)) {
      auto strategies =
          get_strategies(routing_strategies, category_group.first);
      auto const g = build_seq_graph(seq, strategies);

      // for (auto const& nodes : g.station_to_nodes_) {
      //   std::cout << "\nStation "
      //             << (nodes.size() > 0 ? nodes[0]->station_idx_ : -1) << ":
      //             ";
      //   for (auto const& n : nodes) {
      //     std::cout << "\n\t Node " << n->idx_ << ":\n";
      //     for (auto const& e : n->edges_) {
      //       std::cout << "\n(" << n->idx_ << "|" << e.to_->idx_
      //                 << "| w: " << e.weight_ << ") ("
      //                 << e.from_->ref_.router_id_ << "|"
      //                 << e.to_->ref_.router_id_ << ")";
      //     }
      //   }
      // }

      seq_graph_dijkstra dijkstra(g, g.initials_, g.goals_);
      dijkstra.run();

      auto best_goal_it =
          std::min_element(begin(g.goals_), end(g.goals_),
                           [&](auto const& lhs_idx_, auto const& rhs_idx_) {
                             return dijkstra.get_distance(lhs_idx_) <
                                    dijkstra.get_distance(rhs_idx_);
                           });
      if (best_goal_it == end(g.goals_)) {
        continue;
      }
      std::vector<std::vector<geo::latlng>> lines{seq.station_ids_.size() - 1};
      std::vector<sequence_info> sequence_infos;
      for (auto const& edge : dijkstra.get_links(*best_goal_it)) {
        sequence_info info;
        info.idx_ = edge->from_->station_idx_;
        info.from_ = lines[edge->from_->station_idx_].size();
        // std::cout << "\n(" << edge->from_->idx_ << "|" << edge->to_->idx_
        //           << "| w: " << edge->weight_ << ") "
        //           << "(" << edge->from_->ref_.router_id_ << "|"
        //           << edge->to_->ref_.router_id_ << ")";
        verify(edge->from_->ref_.id_.relation_id_ ==
                       edge->to_->ref_.id_.relation_id_ &&
                   edge->router_id() != 0 &&
                   edge->from_->ref_.router_id_ == edge->to_->ref_.router_id_ &&
                   edge->router_id() == edge->from_->ref_.router_id_ &&
                   edge->router_id() == edge->to_->ref_.router_id_,
               "error prepare");
        append(lines[edge->from_->station_idx_],
               routing_strategies.strategies_[edge->router_id()]->get_polyline(
                   edge->from_->ref_, edge->to_->ref_));

        info.to_ = lines[edge->from_->station_idx_].size();
        info.type_ =
            edge->source_spec().type_ == source_spec::type::OSRM_ROUTE
                ? "OSRM"
                : edge->source_spec().type_ == source_spec::type::STUB_ROUTE
                      ? "STUB"
                      : "RELATION";
        sequence_infos.push_back(info);
      }
      builder.append(seq.station_ids_, category_group.second, lines,
                     sequence_infos);
    }
  });

  builder.finish();
  timer.stop_and_print();
}

std::vector<routing_strategy*> get_strategies(
    strategies const& routing_strategies,
    source_spec::category const& category) {
  std::vector<routing_strategy*> strategies;
  strategies.push_back(routing_strategies.relation_strategy_.get());
  strategies.push_back(routing_strategies.stub_strategy_.get());
  return strategies;
}

}  // namespace path
}  // namespace motis