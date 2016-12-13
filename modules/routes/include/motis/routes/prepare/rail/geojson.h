#pragma once

#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"

#include "motis/routes/prepare/rail/rail_graph.h"

using namespace rapidjson;

namespace motis {
namespace routes {

inline void dump_rail_graph(rail_graph const& graph) {
  FILE* fp = std::fopen("rail_graph.json", "w");
  char writeBuffer[65536];

  rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
  rapidjson::PrettyWriter<rapidjson::FileWriteStream> w(os);

  w.StartObject();
  w.String("type").String("FeatureCollection");
  w.String("features").StartArray();

  for (auto const& node : graph.nodes_) {
    for (auto const& link : node->links_) {

      w.StartObject();
      w.String("type").String("Feature");
      w.String("properties").StartObject().EndObject();
      w.String("geometry").StartObject();
      w.String("type").String("LineString");
      w.String("coordinates").StartArray();

      for (auto const& coords : link.polyline_) {
        w.StartArray();
        w.Double(coords.lng_, 9);
        w.Double(coords.lat_, 9);
        w.EndArray();
      }

      w.EndArray();
      w.EndObject();
      w.EndObject();
    }
  }

  w.EndArray();
  w.EndObject();
}

inline void dump_paths(
    std::vector<std::vector<rail_link const*>> const& paths) {
  FILE* fp = std::fopen("rail_paths.json", "w");
  char writeBuffer[65536];

  rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
  rapidjson::PrettyWriter<rapidjson::FileWriteStream> w(os);

  w.StartObject();
  w.String("type").String("FeatureCollection");
  w.String("features").StartArray();

  for (auto const& path : paths) {
    if (path.empty()) {
      continue;
    }

    w.StartObject();
    w.String("type").String("Feature");
    w.String("properties").StartObject().EndObject();
    w.String("geometry").StartObject();
    w.String("type").String("LineString");
    w.String("coordinates").StartArray();

    for (auto const& link : path) {
      for (auto const& coords : simplify(link->polyline_)) {
        w.StartArray();
        w.Double(coords.lng_, 9);
        w.Double(coords.lat_, 9);
        w.EndArray();
      }
    }

    w.EndArray();
    w.EndObject();
    w.EndObject();
  }

  w.EndArray();
  w.EndObject();
}

inline void dump_polylines(std::vector<geo::polyline> const& polylines) {
  FILE* fp = std::fopen("poly.json", "w");
  char writeBuffer[65536];

  rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
  rapidjson::PrettyWriter<rapidjson::FileWriteStream> w(os);

  w.StartObject();
  w.String("type").String("FeatureCollection");
  w.String("features").StartArray();
  for (auto const& polyline : polylines) {
    w.StartObject();
    w.String("type").String("Feature");
    w.String("properties").StartObject().EndObject();
    w.String("geometry").StartObject();
    w.String("type").String("LineString");
    w.String("coordinates").StartArray();

    for (auto const& coords : polyline) {
      w.StartArray();
      w.Double(coords.lng_, 9);
      w.Double(coords.lat_, 9);
      w.EndArray();
    }

    w.EndArray();
    w.EndObject();
    w.EndObject();
  }

  w.EndArray();
  w.EndObject();
}

}  // namespace routes
}  // namespace motis
