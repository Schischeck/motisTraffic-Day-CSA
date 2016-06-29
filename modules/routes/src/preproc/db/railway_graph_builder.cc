#include "motis/routes/preproc/db/railway_graph_builder.h"

#include <fstream>

#include "parser/util.h"

#include "motis/core/common/get_or_create.h"
#include "motis/routes/preproc/geo_util.h"

namespace rapidjson {
template <typename Encoding, typename Allocator>
typename GenericValue<Encoding, Allocator>::ValueIterator begin(
    GenericValue<Encoding, Allocator>& v) {
  return v.Begin();
}
template <typename Encoding, typename Allocator>
typename GenericValue<Encoding, Allocator>::ConstValueIterator begin(
    const GenericValue<Encoding, Allocator>& v) {
  return v.Begin();
}

template <typename Encoding, typename Allocator>
typename GenericValue<Encoding, Allocator>::ValueIterator end(
    GenericValue<Encoding, Allocator>& v) {
  return v.End();
}
template <typename Encoding, typename Allocator>
typename GenericValue<Encoding, Allocator>::ConstValueIterator end(
    const GenericValue<Encoding, Allocator>& v) {
  return v.End();
}
}  // namespace rapidjson

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

  finalize_links();

  std::cout << graph_.nodes_.size() << std::endl;
  std::cout << graph_.links_.size() << std::endl;
}

void railway_graph_builder::read_nodes(rapidjson::Document const& doc) {
  for (auto const& feature : doc["features"]) {
    auto const& geometry = feature["geometry"]["coordinates"];
    auto pos = coord{geometry[1u].GetDouble(), geometry[0u].GetDouble()};

    auto const& properties = feature["properties"];
    std::string id = properties["id"].GetString();
    std::string ds100 = "";
    if (properties.HasMember("railwayStationCode")) {
      ds100 = properties["railwayStationCode"].GetString();
    }

    auto node =
        std::make_unique<railway_node>(graph_.nodes_.size(), id, pos, ds100);

    add_links(properties["spokeStartIds"], *node);
    add_links(properties["spokeEndIds"], *node);
    graph_.ds100_to_node_.insert(std::make_pair(node->ds100_, node.get()));
    graph_.nodes_.push_back(std::move(node));
  }
}

void railway_graph_builder::add_links(rapidjson::Value const& v,
                                      railway_node& node) {
  for (rapidjson::SizeType i = 0; i < v.Size(); i++) {
    std::string link_id(v[i].GetString());
    get_or_create(raw_links_, link_id, [&]() {
      return std::set<uint32_t>{};
    }).insert(node.idx_);
  }
}

void railway_graph_builder::read_links(rapidjson::Document const& doc) {
  for (auto const& feature : doc["features"]) {
    std::vector<coord> polyline;
    for (auto const& c : feature["geometry"]["coordinates"]) {
      polyline.emplace_back(c[1u].GetDouble(), c[0u].GetDouble());
    }

    auto const& properties = feature["properties"];
    raw_polylines_.emplace(properties["id"].GetString(),
                           std::make_pair(properties["startNodeId"].GetString(),
                                          std::move(polyline)));
  }
}

void railway_graph_builder::finalize_links() {
  for (auto const& pair : raw_links_) {
    verify(pair.second.size() == 2, "railway_link: invalid node count");
    auto const a = *begin(pair.second);
    auto const b = *std::next(begin(pair.second));

    finalize_link(pair.first, a, b);
    finalize_link(pair.first, b, a);
  }
}

void railway_graph_builder::finalize_link(std::string const& id,
                                          uint32_t const from,
                                          uint32_t const to) {
  auto const& from_node = graph_.nodes_[from].get();
  auto const& raw_polyline = raw_polylines_.at(id);
  auto const& length = get_length(raw_polyline.second);

  auto from_pos = from_node->pos_;
  auto start_pos = raw_polyline.second.front();
  auto end_pos = raw_polyline.second.back();

  auto polyline = raw_polyline.second;
  if (geo_detail::distance_in_m({from_pos.lng_, from_pos.lat_},
                                {start_pos.lng_, start_pos.lat_}) >
      geo_detail::distance_in_m({from_pos.lng_, from_pos.lat_},
                                {end_pos.lng_, end_pos.lat_})) {
    std::reverse(begin(polyline), end(polyline));
  }

  // if (from_node->pos_.lat_ != raw_polyline.second[0].lat_ &&
  //     from_node->pos_.lng_ != raw_polyline.second[0].lng_) {
  // }

  auto link = std::make_unique<railway_link>(id, polyline, length, from_node,
                                             graph_.nodes_[to].get());

  from_node->links_.push_back(link.get());
  graph_.links_.push_back(std::move(link));
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
