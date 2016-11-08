#include "motis/routes/prepare/prepare_data.h"

#include "motis/core/common/logging.h"
#include "motis/routes/db/db_builder.h"

#include "motis/routes/db/kv_database.h"
#include "motis/routes/prepare/bus_stop_positions.h"
#include "motis/routes/prepare/geojson.h"
#include "motis/routes/prepare/parallel_for.h"
#include "motis/routes/prepare/rel/relation_matcher.h"
#include "motis/routes/prepare/seq/seq_graph_builder.h"
#include "motis/routes/prepare/seq/seq_graph_dijkstra.h"
#include "motis/routes/prepare/station_sequences.h"
#include "motis/routes/prepare/vector_utils.h"

#include "version.h"

using namespace motis;
using namespace motis::loader;
using namespace motis::routes;
using namespace geo;

namespace motis {
namespace routes {

void prepare(
    std::vector<station_seq>& sequences,
    std::map<std::string, std::vector<geo::latlng>> const& stop_positions,
    std::string const& osm, kv_database& db) {
  motis::logging::manual_timer timer("preparing data");
  auto const rel_matches = match_osm_relations(osm, sequences, stop_positions);
  db_builder builder(db);
  std::vector<std::pair<station_seq, std::vector<match_seq>>> results;
  for (auto i = 0u; i < sequences.size(); ++i) {
    results.emplace_back(sequences[i], rel_matches[i]);
  }
  parallel_for("searching routes", results, 250, [&](auto const& r) {
    auto const& seq = r.first;
    auto const& relations = r.second;

    for (auto const& category_group : category_groups(seq.categories_)) {
      stub_routing strategy{seq};
      auto const g =
          build_seq_graph(category_group.first, seq, relations, strategy);

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
        append(lines[edge->from_->station_idx_], edge->p_);
      }

      builder.append(seq.station_ids_, category_group.second, lines);
    }
  });

  builder.finish();
  timer.stop_and_print();
}

}  // namespace routes
}  // namespace motis