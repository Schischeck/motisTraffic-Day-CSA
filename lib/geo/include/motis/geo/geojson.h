#pragma once

#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"

#include "motis/geo/polyline.h"

namespace motis {
namespace geo {

inline void dump_polylines(std::vector<polyline> const& polylines,
                           const char* filename = "polylines.json") {
  FILE* fp = std::fopen(filename, "w");
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

}  // namespace geo
}  // namespace motis
