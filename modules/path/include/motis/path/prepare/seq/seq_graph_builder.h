#pragma once

#include <vector>

#include "motis/path/prepare/schedule/station_sequences.h"
#include "motis/path/prepare/seq/seq_graph.h"
#include "motis/path/prepare/strategy/routing_strategy.h"

namespace motis {
namespace path {

seq_graph build_seq_graph(station_seq const&,
                          std::vector<routing_strategy*> const&);

void dump_build_seq_graph_timings();

}  // namespace path
}  // namespace motis
