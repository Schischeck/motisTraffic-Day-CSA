#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "rapidjson/document.h"

#include "motis/core/common/logging.h"
#include "motis/core/schedule/schedule.h"
#include "motis/routes/preproc/db/railway_graph.h"

namespace motis {
namespace routes {

class railway_graph_builder {
public:
  railway_graph_builder(railway_graph& graph);

  void build_graph(std::string root);

private:
  void read_nodes(rapidjson::Document const& doc);
  void add_links(const rapidjson::Value& v, railway_node& n);

  void read_links(rapidjson::Document const& doc);
  void finalize_links();
  void finalize_link(std::string const& id, uint32_t const from,
                     uint32_t const to);

  void read_file(std::string const& filename, rapidjson::Document& doc);

  railway_graph& graph_;

  std::map<std::string, std::set<uint32_t>> raw_links_;
  std::map<std::string, std::pair<std::string, std::vector<coord>>>
      raw_polylines_;
};

}  // namespace routes
}  // namespace motis
