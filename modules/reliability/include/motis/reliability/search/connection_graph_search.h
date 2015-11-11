#pragma once

#include <memory>
#include <vector>

#include "motis/module/module.h"

#include "motis/reliability/reliability.h"
#include "motis/reliability/search/cg_search_callback.h"
#include "motis/reliability/search/connection_graph.h"

namespace motis {
namespace reliability {
struct ReliableRoutingRequest;
namespace search {
namespace connection_graph_search {
struct connection_graph_optimizer;

void search_cgs(ReliableRoutingRequest const*, reliability&, motis::module::sid,
                std::shared_ptr<connection_graph_optimizer const>, callback);

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
