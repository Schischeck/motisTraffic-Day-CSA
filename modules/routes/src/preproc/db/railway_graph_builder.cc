#include "motis/routes/preproc/db/railway_graph_builder.h"

#include <fstream>

#include "parser/util.h"

#include "motis/core/common/get_or_create.h"
#include "motis/routes/preproc/geo_util.h"

using namespace motis::geo_detail;


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
  for (auto const& f : doc["features"]) {
    auto polyline = std::make_unique<std::vector<coord>>();
    for (auto const& c : f["geometry"]["coordinates"]) {
      polyline->emplace_back(c[1u].GetDouble(), c[0u].GetDouble());
    }

    raw_polylines_.emplace(f["properties"]["id"].GetString(), polyline.get());
    graph_.polylines_.emplace_back(std::move(polyline));
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
  auto polyline = raw_polylines_.at(id);
  auto length = static_cast<size_t>(get_length(*polyline));

  auto from_node = get_node(from, polyline);
  auto to_node = get_node(to, polyline);

  from_node->links_.push_back({id, polyline, length, from_node, to_node});
}

railway_node* railway_graph_builder::get_node(
    uint32_t const node_id, std::vector<coord> const* polyline) {
  auto node = graph_.nodes_[node_id].get();
  auto const& start_pos = polyline->front();
  auto const& end_pos = polyline->back();

  auto const start_dist = distance_in_m(node->pos_.lat_, node->pos_.lng_,
                                        start_pos.lat_, start_pos.lng_);
  auto const end_dist = distance_in_m(node->pos_.lat_, node->pos_.lng_,
                                      end_pos.lat_, end_pos.lng_);

  // only attach the link to this node if the gap is reasonably small
  if (std::min(start_dist, end_dist) < 10) {
    return node;
  } else if (node->extra_ != nullptr) {
    return get_node(node->extra_->idx_, polyline);
  } else {
    return make_extra_node(node, start_dist < end_dist ? start_pos : end_pos);
  }
}

railway_node* railway_graph_builder::make_extra_node(
    railway_node* node, coord const& pos) {
  auto new_node = std::make_unique<railway_node>(
      graph_.nodes_.size(), node->id_ + "-extra", pos, node->ds100_);
  node->extra_ = new_node.get();
  // TODO global DS100 Mapping
  graph_.nodes_.push_back(std::move(new_node));
  return graph_.nodes_.back().get();
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
