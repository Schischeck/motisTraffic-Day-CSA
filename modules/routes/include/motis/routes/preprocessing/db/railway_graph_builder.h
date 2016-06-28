#pragma once

#include <map>
#include <string>
#include <vector>

#include "rapidjson/document.h"

#include "motis/core/common/logging.h"
#include "motis/core/schedule/schedule.h"
#include "motis/routes/preprocessing/db/railway_graph.h"

namespace motis {
namespace routes {

class railway_graph_builder {
public:
  railway_graph_builder(railway_graph& graph);

  void build_graph(std::string root);

private:
  void get_link(const rapidjson::Value& v, railway_node& n);
  void read_nodes(rapidjson::Document const& doc);
  void read_links(rapidjson::Document const& doc);
  void read_file(std::string const& filename, rapidjson::Document& doc);

  railway_graph& graph_;
};

}  // namespace routes
}  // namespace motis
