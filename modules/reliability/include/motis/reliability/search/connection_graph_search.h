#pragma once

#include <memory>
#include <vector>

#include "motis/module/module.h"

#include "motis/reliability/reliability.h"

namespace motis {
namespace reliability {
struct ReliableRoutingRequest;  // NOLINT
struct context;
namespace search {
struct connection_graph;
namespace connection_graph_search {
struct connection_graph_optimizer;

std::vector<std::shared_ptr<connection_graph> > search_cgs(
    ReliableRoutingRequest const&, motis::reliability::context const&,
    std::shared_ptr<connection_graph_optimizer const>);

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
