#pragma once

#include <memory>
#include <vector>

#include "motis/module/module.h"

#include "motis/reliability/search/connection_graph.h"

namespace motis {
namespace reliability {
struct reliability;
struct ReliableRoutingRequest;
namespace search {
namespace connection_graph_search {
typedef std::function<bool(connection_graph::stop const&,
                           connection_graph const&)> complete_func;
typedef std::function<void(
    std::vector<std::shared_ptr<connection_graph> > const)> callback;

void search_cgs(ReliableRoutingRequest const*, reliability&, motis::module::sid,
                complete_func, callback);

}  // namespace connection_graph_search
}  // namespace search
}  // namespace reliability
}  // namespace motis
