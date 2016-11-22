#pragma once

#include "motis/routes/prepare/rel/relation_matcher.h"
#include "motis/routes/prepare/seq/routing_strategy.h"
#include "motis/routes/prepare/seq/seq_graph.h"
#include "motis/routes/prepare/station_sequences.h"

namespace motis {
namespace routes {

struct strategies;

seq_graph build_seq_graph(source_spec::category const&, station_seq const&,
                          std::vector<match_seq> const&,
                          strategies& routing_strategies);

}  // namespace routes
}  // namespace motis
