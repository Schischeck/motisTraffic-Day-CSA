#pragma once

#include <memory>
#include <vector>

namespace motis {
namespace reliability {
namespace search {
struct connection_graph;
namespace connection_graph_search {
typedef std::function<void(
    std::vector<std::shared_ptr<connection_graph> > const)> callback;
}
}
}
}
