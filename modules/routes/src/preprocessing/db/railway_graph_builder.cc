#include "motis/routes/preprocessing/db/railway_graph_builder.h"

#include <fstream>

#include "motis/core/common/get_or_create.h"
#include "motis/routes/preprocessing/geo_util.h"

namespace motis {
namespace routes {

railway_graph_builder::railway_graph_builder(railway_graph& graph)
    : graph_(graph){};

void railway_graph_builder::build_graph(std::string root) {
  motis::logging::scoped_timer timer("Building Graph");

  for (auto file : {root + "/railwayStationNodes.geojson",
                    root + "/railwayNodes.geojson"}) {
    rapidjson::Document doc;
    read_file(file, doc);
    read_nodes(doc);
  }

  std::string link_file(root + "/railwayLinks.geojson");
  rapidjson::Document railway_links;
  read_file(link_file, railway_links);
  read_links(railway_links);

  std::cout << graph_.nodes_.size() << std::endl;
  std::cout << graph_.links_.size() << std::endl;
}

void railway_graph_builder::read_nodes(rapidjson::Document const& doc) {
  auto const& features = doc["features"];
  for (rapidjson::SizeType i = 0; i < features.Size(); i++) {
    auto const& geometry = features[i]["geometry"]["coordinates"];
    auto const& properties = features[i]["properties"];
    auto const& start = properties["spokeStartIds"];
    auto const& end = properties["spokeEndIds"];

    double lat = geometry[0u].GetDouble();
    double lng = geometry[1u].GetDouble();

    std::string ds100 = "";
    if (properties.HasMember("railwayStationCode")) {
      ds100 = properties["railwayStationCode"].GetString();
    }

    std::string id = properties["id"].GetString();
    auto node = std::make_unique<railway_node>(id, lat, lng, ds100);

    get_link(start, *node);
    get_link(end, *node);
    graph_.ds100_to_node_.insert(std::make_pair(node->ds100_, node.get()));
    graph_.nodes_.push_back(std::move(node));
  }
}

void railway_graph_builder::get_link(rapidjson::Value const& v,
                                     railway_node& n) {
  for (rapidjson::SizeType i = 0; i < v.Size(); i++) {
    std::string link_id(v[i].GetString());
    get_or_create(graph_.links_, link_id, [&]() {
      return std::make_unique<railway_link>(link_id, 0);
    })->nodes_.push_back(&n);
    n.links_.push_back(graph_.links_.at(link_id).get());
  }
}

void railway_graph_builder::read_links(rapidjson::Document const& doc) {
  auto const& features = doc["features"];
  for (rapidjson::SizeType i = 0; i < features.Size(); i++) {
    auto const& geometry = features[i]["geometry"]["coordinates"];
    auto const& properties = features[i]["properties"];
    auto& link = *graph_.links_.at(properties["id"].GetString());
    for (rapidjson::SizeType j = 0; j < geometry.Size(); j++) {
      link.coords_.push_back(geometry[j][0u].GetDouble());
      link.coords_.push_back(geometry[j][1u].GetDouble());
    }
    // link.weight_ = get_length(link.coords_);
  }
}

void railway_graph_builder::read_file(std::string const& filename,
                                      rapidjson::Document& doc) {
  // TODO(Jonas) rewrite file import
  std::string line;
  std::string content;
  std::ifstream fs(filename);
  while (getline(fs, line)) {
    content += line + "\n";
  }
  auto json = content.c_str();
  doc.Parse<0>(json);
}

}  // namespace routes
}  // namespace motis
