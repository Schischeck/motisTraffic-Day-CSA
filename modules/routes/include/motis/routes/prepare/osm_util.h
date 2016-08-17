#pragma once

#include <algorithm>

#include "osmium/handler.hpp"
#include "osmium/io/pbf_input.hpp"
#include "osmium/io/reader_iterator.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/memory/buffer.hpp"
#include "osmium/osm.hpp"
#include "osmium/visitor.hpp"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"

#include "motis/core/common/geo.h"

using namespace rapidjson;

namespace motis {
namespace routes {

template <typename F>
void foreach_osm_node(std::string const& filename, F f) {
  osmium::io::Reader reader(filename, osmium::osm_entity_bits::node);
  for (auto it = std::begin(reader); it != std::end(reader); ++it) {
    f(static_cast<osmium::Node&>(*it));
  }
}

template <typename F>
void foreach_osm_way(std::string const& filename, F f) {
  osmium::io::Reader reader(filename, osmium::osm_entity_bits::way);
  for (auto it = std::begin(reader); it != std::end(reader); ++it) {
    f(static_cast<osmium::Way&>(*it));
  }
}

template <typename F>
void foreach_osm_relation(std::string const& filename, F f) {
  osmium::io::Reader reader(filename, osmium::osm_entity_bits::relation);
  for (auto it = std::begin(reader); it != std::end(reader); ++it) {
    f(static_cast<osmium::Relation&>(*it));
  }
}

struct osm_node {
  osm_node(int64_t id) : id_(id) {}

  int64_t id_;
  osmium::Location location_;
};

struct osm_relation {
  osm_relation(int64_t id) : id_(id) {}

  std::vector<int64_t> nodes_;
  std::vector<int64_t> ways_;
  bool broken = false;
  int64_t id_;
};

struct osm_way {
  osm_way(int64_t id) : id_(id) {}

  std::vector<int64_t> nodes_;
  int64_t id_;
};

struct way {
  way(int64_t id, int64_t front, int64_t back)
      : id_(id), front_(front), back_(back) {}
  int64_t id_;
  int64_t front_;
  int64_t back_;
  osmium::Location f;
  osmium::Location b;
};

inline std::vector<way> expand_segment(
    motis::routes::way const& way,
    std::vector<motis::routes::way> const& ways) {
  std::vector<motis::routes::way> seg = {way};
  std::vector<int64_t> used = {way.id_};
  while (used.size() < ways.size()) {
    std::vector<motis::routes::way> add_front;
    std::vector<motis::routes::way> add_back;
    auto& front = seg.front().front_;
    auto& back = seg.back().back_;
    for (auto const& w : ways) {
      if (std::find(begin(used), end(used), w.id_) == end(used)) {
        if (w.back_ == front) {
          add_front.push_back(w);
          used.push_back(w.id_);
        }
        if (w.front_ == back) {
          add_back.push_back(w);
          used.push_back(w.id_);
        }
      }
    }
    if (add_back.size() > 1 || add_front.size() > 1) {
      return {};
    }
    if (add_back.size() == 1) {
      seg.push_back(add_back.front());
    }
    if (add_front.size() == 1) {
      seg.insert(begin(seg), add_front.front());
    }
    if (add_front.empty() && add_back.empty()) {
      break;
    }
  }
  return seg;
}

inline std::vector<int64_t> sort_segments(
    std::vector<std::vector<way>> segments) {
  std::vector<int64_t> result;
  auto start = segments.front();
  bool add_front = false;
  double max_dist = 10;
  segments.erase(begin(segments));
  while (!segments.empty()) {
    auto const loc = add_front ? start.front().f : start.back().f;
    std::vector<std::pair<std::size_t, bool>> add;
    for (std::size_t i = 0; i < segments.size(); i++) {
      auto const& f_dist = geo_detail::distance_in_m(
          segments[i].front().f.lat(), segments[i].front().f.lon(), loc.lat(),
          loc.lon());
      auto const& b_dist = geo_detail::distance_in_m(segments[i].back().f.lat(),
                                                     segments[i].back().f.lon(),
                                                     loc.lat(), loc.lon());
      if (f_dist < max_dist) {
        add.emplace_back(i, false);
      } else if (b_dist < max_dist) {
        add.emplace_back(i, true);
      }
    }
    if (add.size() > 1 || (add_front && add.empty())) {
      return {};
    }
    if (add.empty()) {
      add_front = true;
    } else {
      auto s = segments[add[0].first];
      if (add[0].second) {
        std::reverse(begin(s), end(s));
      }
      if (add_front) {
        start.insert(begin(start), begin(s), end(s));
      } else {
        start.insert(end(start), begin(s), end(s));
      }
      segments.erase(begin(segments) + add[0].first);
    }
  }
  for (auto const& s : start) {
    result.push_back(s.id_);
  }
  return result;
}

inline std::vector<int64_t> sort_ways(std::vector<way>& ways) {
  std::vector<std::vector<way>> segments;
  if (ways.empty()) {
    return {};
  }
  while (!ways.empty()) {
    auto& start = ways[0];
    auto const& seg = expand_segment(start, ways);
    if (seg.empty()) {
      return {};
    }
    for (auto const& s : seg) {
      auto w = std::find_if(begin(ways), end(ways),
                            [s](auto&& w) { return w.id_ == s.id_; });
      if (w != end(ways)) {
        ways.erase(w);
      }
    }
    segments.push_back(seg);
  }
  return sort_segments(segments);
}

inline void prepare_relations(std::map<int64_t, osm_relation>& relations,
                              std::map<int64_t, osm_way> const& ways,
                              std::map<int64_t, osm_node>& nodes) {
  std::cout << std::endl;
  std::vector<int64_t> broken_relations;
  int count = 0;
  for (auto& rel : relations) {
    std::cout << count << "/" << relations.size() << "\r";
    std::vector<way> small_ways;
    count++;
    for (auto const& w : rel.second.ways_) {
      auto const way = ways.find(w);
      if (way != ways.end() && !way->second.nodes_.empty()) {
        small_ways.emplace_back(way->second.id_, way->second.nodes_.front(),
                                way->second.nodes_.back());
        auto f = nodes.find(small_ways.back().front_);
        auto b = nodes.find(small_ways.back().back_);
        if (f != end(nodes) && b != end(nodes)) {
          small_ways.back().f = f->second.location_;
          small_ways.back().b = b->second.location_;
        }
      }
    }
    auto const& sorted_ways = sort_ways(small_ways);
    if (sorted_ways.empty()) {
      broken_relations.push_back(rel.first);
      rel.second.broken = true;
      continue;
    }
    rel.second.ways_.clear();
    for (auto const& w : sorted_ways) {
      auto way = ways.find(w);
      if (way != end(ways)) {
        rel.second.ways_.push_back(w);
        rel.second.nodes_.insert(rel.second.nodes_.end(),
                                 way->second.nodes_.begin(),
                                 way->second.nodes_.end());
      }
    }
  }
  std::cout << "Broken relations: " << broken_relations.size() << std::endl;
}

inline void export_geojson(std::vector<osm_relation> const& rels,
                           std::map<int64_t, osm_way> const& ways,
                           std::map<int64_t, osm_node> const& nodes) {
  std::cout << "EXPORT: " << rels.size() << std::endl;
  FILE* fp = std::fopen("geo.json", "w");
  char writeBuffer[65536];

  rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
  rapidjson::Writer<rapidjson::FileWriteStream> w(os);

  w.StartObject();
  w.String("type").String("FeatureCollection");
  w.String("features").StartArray();

  for (auto const& rel : rels) {
    // for (auto const& goal : goal_idx) {
    w.StartObject();
    w.String("type").String("Feature");
    w.String("properties").StartObject().EndObject();
    w.String("geometry").StartObject();
    w.String("type").String("LineString");
    w.String("coordinates").StartArray();

    for (auto const& way : rel.ways_) {
      auto wy = ways.find(way);
      if (wy != end(ways)) {
        for (auto const& n : wy->second.nodes_) {
          auto node = nodes.find(n);
          if (node != end(nodes)) {
            w.StartArray();
            w.Double(node->second.location_.lon(), 9);
            w.Double(node->second.location_.lat(), 9);
            w.EndArray();
          }
        }
      }
    }
    w.EndArray();
    w.EndObject();
    w.EndObject();
  }

  w.EndArray();
  w.EndObject();
}
struct segment_match {
  segment_match(int64_t rel, std::vector<std::string> stations,
                std::vector<int64_t> station_nodes)
      : rel_(rel), stations_(stations), station_nodes_(station_nodes){};

  int64_t rel_;
  std::vector<std::string> stations_;
  std::vector<int64_t> station_nodes_;
};
}
}
