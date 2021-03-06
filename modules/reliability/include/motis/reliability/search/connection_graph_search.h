#pragma once

#include <memory>
#include <vector>

#include "motis/module/module.h"

#include "motis/reliability/reliability.h"

namespace motis {
namespace reliability {
struct reliability;
struct ReliableRoutingRequest;  // NOLINT
struct context;
namespace intermodal {
struct individual_modes_container;
}  // namespace intermodal
namespace search {
struct connection_graph;
namespace connection_graph_search {
struct connection_graph_optimizer;

module::msg_ptr search_cgs(ReliableRoutingRequest const&, reliability&,
                           unsigned const max_bikesharing_duration,
                           bool const pareto_filtering_for_bikesharing);

std::vector<std::shared_ptr<connection_graph>> search_cgs(
    ReliableRoutingRequest const&, motis::reliability::context const&,
    std::shared_ptr<connection_graph_optimizer const>,
    intermodal::individual_modes_container const&);

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
