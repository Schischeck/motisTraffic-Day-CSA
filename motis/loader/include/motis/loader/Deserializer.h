#pragma once

#include <memory>
#include <vector>
#include <string>

#include "motis/core/common/array.h"
#include "motis/core/common/offset.h"
#include "motis/core/schedule/connection.h"
#include "motis/core/schedule/station.h"
#include "motis/core/schedule/nodes.h"

namespace td {

class deserializer {
public:
  deserializer(std::string const& prefix);

  std::pair<int, std::unique_ptr<char[]>> load_graph(
      std::vector<station_ptr>& stations,
      std::vector<station_node_ptr>& station_nodes);

private:
  std::string _prefix;
};

}  // namespace td
