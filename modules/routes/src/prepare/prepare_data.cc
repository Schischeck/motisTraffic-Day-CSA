#include "motis/routes/prepare/prepare_data.h"

#include "motis/core/common/logging.h"

#include "motis/routes/db/db_builder.h"
#include "motis/routes/db/kv_database.h"
#include "motis/routes/prepare/parallel_for.h"
#include "motis/routes/prepare/rel/relation_matcher.h"
#include "motis/routes/prepare/seq/seq_graph_builder.h"
#include "motis/routes/prepare/seq/seq_graph_dijkstra.h"
#include "motis/routes/prepare/station_sequences.h"
#include "motis/routes/prepare/vector_utils.h"

using namespace motis;
using namespace motis::routes;

namespace motis {
namespace routes {

void prepare(prepare_data& data, strategies& routing_strategies,
             kv_database& db, std::string const& osm) {
  motis::logging::manual_timer timer("preparing data");
  auto const rel_matches =
      match_osm_relations(osm, data.sequences_, data.stop_positions_);
  db_builder builder(db);
  std::vector<std::pair<station_seq, std::vector<match_seq>>> results;
  for (auto i = 0u; i < 2; ++i) {
    results.emplace_back(data.sequences_[i], rel_matches[i]);
  }
  parallel_for("searching routes", results, 250, [&](auto const& r) {
    auto const& seq = r.first;
    auto const& relations = r.second;

    for (auto const& category_group : category_groups(seq.categories_)) {
      auto const g = build_seq_graph(category_group.first, seq, relations,
                                     routing_strategies);
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
      for (auto const& edge : dijkstra.get_links(*best_goal_it)) {
        if (edge->source_.type_ == source_spec::type::ROUTE) {
          routing_strategies.strategies_[edge->source_.router_id_]
              ->get_polyline(edge->from_->ref_, edge->to_->ref_);
        }
        append(lines[edge->from_->station_idx_], edge->p_);
      }
      builder.append(seq.station_ids_, category_group.second, lines);
    }
  });

  builder.finish();
  timer.stop_and_print();
}

routing_strategy* get_strategy(strategies const& routing_strategies,
                               source_spec::category const& category) {
  auto it = routing_strategies.class_to_strategy_.find(category);
  if (it == end(routing_strategies.class_to_strategy_)) {
    return routing_strategies.strategies_[0].get();
  }
  return it->second;
}

}  // namespace routes
}  // namespace motis