#pragma once

#include "motis/path/prepare/rel/relation_matcher.h"
#include "motis/path/prepare/routing/routing_strategy.h"
#include "motis/path/prepare/seq/seq_graph.h"
#include "motis/path/prepare/schedule/station_sequences.h"

namespace motis {
namespace path {

struct strategies;

seq_graph build_seq_graph(source_spec::category const&, station_seq const&,
                          std::vector<match_seq> const&,
                          strategies& routing_strategies);

seq_graph build_seq_graph(
    station_seq const& seq,
    std::vector<routing_strategy*> const& routing_strategies);

}  // namespace path
}  // namespace motis
