#pragma once

#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"

#include "geo/polyline.h"

namespace motis {
namespace path {

inline void dump_polylines(std::vector<geo::polyline> const& polylines,
                           const char* filename = "polylines.json") {
  FILE* fp = std::fopen(filename, "w");
  char write_buffer[65536];

  rapidjson::FileWriteStream os(fp, write_buffer, sizeof(write_buffer));
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

}  // namespace path
}  // namespace motis
