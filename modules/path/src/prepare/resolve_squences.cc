#include "motis/path/prepare/resolve_sequences.h"

#include "parser/util.h"

#include "utl/concat.h"
#include "utl/parallel_for.h"

#include "motis/core/common/logging.h"

#include "motis/path/db/kv_database.h"

#include "motis/path/prepare/seq/seq_graph_printer.h"
#include "motis/path/prepare/seq/seq_graph_builder.h"
#include "motis/path/prepare/seq/seq_graph_dijkstra.h"

namespace motis {
namespace path {

void resolve_sequences(std::vector<station_seq> const& sequences,
                       path_routing& routing, db_builder& builder) {
  motis::logging::scoped_timer timer("resolve_sequences");

  utl::parallel_for("searching routes", sequences, 250, [&](auto const& seq) {
    for (auto const& category_group : category_groups(seq.categories_)) {
      auto strategies = routing.strategies_for();  // (category_group.first);

      auto const get_polyline = [&strategies](seq_edge const* edge) {
        auto const it = std::find_if(
            begin(strategies), end(strategies), [&edge](auto const& s) {
              return s->strategy_id() == edge->router_id();
            });
        verify(it != end(strategies), "bad router id");
        return (*it)->get_polyline(edge->from_->ref_, edge->to_->ref_);
      };

      auto const g = build_seq_graph(seq, strategies);
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

      std::vector<geo::polyline> lines{seq.station_ids_.size() - 1};
      std::vector<sequence_info> sequence_infos;

      // print_seq_path(dijkstra.get_links(*best_goal_it));

      for (auto const& edge : dijkstra.get_links(*best_goal_it)) {
        auto& line = lines[edge->from_->station_idx_];

        auto const size_before = line.size();
        utl::concat(line, get_polyline(edge));

        sequence_infos.emplace_back(edge->from_->station_idx_,  //
                                    size_before, line.size(),
                                    edge->source_spec().type_str());
      }
      builder.append(seq.station_ids_, category_group.second, lines,
                     sequence_infos);
    }
  });
}

}  // namespace path
}  // namespace motis
