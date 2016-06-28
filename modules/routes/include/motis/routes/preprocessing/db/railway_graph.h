#pragma once

#include "motis/routes/preprocessing/db/railway_link.h"
#include "motis/routes/preprocessing/db/railway_node.h"

namespace motis {
namespace routes {

class railway_graph {
public:
  std::vector<std::unique_ptr<railway_node>> nodes_;
  std::map<std::string, std::unique_ptr<railway_link>> links_;
  std::map<std::string, railway_node*> ds100_to_node_;
};
}
}
