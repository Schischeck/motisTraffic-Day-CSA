#pragma once

#include "motis/routes/prepare/rel/polyline_aggregator.h"
#include "motis/routes/prepare/rel/relation_parser.h"

#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"

using namespace rapidjson;

namespace motis {
namespace routes {

inline void write_geojson(
    std::vector<std::vector<latlng>> const& polylines) {
  FILE* fp = std::fopen("geo.json", "w");
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

inline void do_something(std::string const& osm_file) {

  auto relations = parse_relations(osm_file);

  auto polylines = aggregate_polylines(relations.relations_);

  write_geojson(polylines);
}

}  // namespace routes
}  // namespace motis
