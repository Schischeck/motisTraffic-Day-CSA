#pragma once

#include <vector>

#include "motis/path/prepare/routing/routing_strategy.h"
#include "motis/path/prepare/schedule/station_sequences.h"
#include "motis/path/prepare/seq/seq_graph.h"

namespace motis {
namespace path {

seq_graph build_seq_graph(station_seq const&,
                          std::vector<routing_strategy*> const&);

}  // namespace path
}  // namespace motis
