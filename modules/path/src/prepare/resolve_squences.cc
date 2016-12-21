#include "motis/path/prepare/resolve_sequences.h"

#include "parser/util.h"

#include "utl/concat.h"
#include "utl/parallel_for.h"

#include "motis/core/common/logging.h"

#include "motis/path/db/kv_database.h"

#include "motis/path/prepare/seq/seq_graph_builder.h"
#include "motis/path/prepare/seq/seq_graph_dijkstra.h"
#include "motis/path/prepare/seq/seq_graph_printer.h"

namespace motis {
namespace path {

void resolve_sequences(std::vector<station_seq> const& sequences,
                       path_routing& routing, db_builder& builder) {
  motis::logging::scoped_timer timer("resolve_sequences");

  utl::parallel_for("resolve sequences", sequences, 100, [&](auto const& seq) {
    foreach_path_category(seq.categories_, [&](auto const& path_category,
                                               auto const& motis_categories) {
      auto strategies = routing.strategies_for(path_category);

      auto const get_polyline = [&strategies](seq_edge const* edge) {
        auto const it = std::find_if(
            begin(strategies), end(strategies), [&edge](auto const& s) {
              return s->strategy_id() == edge->strategy_id();
            });
        verify(it != end(strategies), "bad router id");
        return (*it)->get_polyline(edge->from_->ref_, edge->to_->ref_);
      };

      std::vector<geo::polyline> lines{seq.station_ids_.size() - 1};
      std::vector<sequence_info> sequence_infos;

      auto const graph = build_seq_graph(seq, strategies);
      for (auto const& edge : find_shortest_path(graph)) {
        auto& line = lines[edge->from_->station_idx_];

        auto const size_before = line.size();
        utl::concat(line, get_polyline(edge));

        sequence_infos.emplace_back(edge->from_->station_idx_,  //
                                    size_before, line.size(),
                                    edge->source_spec().type_str());
      }

      builder.append(seq.station_ids_, motis_categories, lines, sequence_infos);
    });
  });
}

}  // namespace path
}  // namespace motis
