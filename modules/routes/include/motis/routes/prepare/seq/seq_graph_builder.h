#pragma once

#include "motis/routes/prepare/rel/relation_matcher.h"
#include "motis/routes/prepare/seq/routing_strategy.h"
#include "motis/routes/prepare/seq/seq_graph.h"
#include "motis/routes/prepare/station_sequences.h"

namespace motis {
namespace routes {

seq_graph build_seq_graph(station_seq const&, std::vector<match_seq> const&,
                          routing_strategy&);

}  // namespace routes
}  // namespace motis
