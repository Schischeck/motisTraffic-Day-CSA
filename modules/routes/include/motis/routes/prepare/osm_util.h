#pragma once

#include "osmium/handler.hpp"
#include "osmium/io/pbf_input.hpp"
#include "osmium/io/reader_iterator.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/memory/buffer.hpp"
#include "osmium/osm.hpp"
#include "osmium/visitor.hpp"

namespace motis {
namespace routes {

template <typename F> void foreach_osm_node(std::string const &filename, F f) {
  osmium::io::Reader reader(filename, osmium::osm_entity_bits::node);
  for (auto it = std::begin(reader); it != std::end(reader); ++it) {
    f(static_cast<osmium::Node &>(*it));
  }
}

template <typename F> void foreach_osm_way(std::string const &filename, F f) {
  osmium::io::Reader reader(filename, osmium::osm_entity_bits::way);
  for (auto it = std::begin(reader); it != std::end(reader); ++it) {
    f(static_cast<osmium::Way &>(*it));
  }
}

template <typename F>
void foreach_osm_relation(std::string const &filename, F f) {
  osmium::io::Reader reader(filename, osmium::osm_entity_bits::relation);
  for (auto it = std::begin(reader); it != std::end(reader); ++it) {
    f(static_cast<osmium::Relation &>(*it));
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
};

inline std::vector<int64_t> sort_ways(std::vector<way> &ways) {
  std::vector<int64_t> visited;
  std::vector<int64_t> result;
  std::vector<std::vector<way>> segments;
  segments.push_back({ways[0]});
  visited.push_back(ways[0].id_);
  while (visited.size() != ways.size()) {
    auto &currentway = segments.back();
    auto front = currentway.front().front_;
    auto back = currentway.back().back_;
    std::vector<way> possible_front;
    std::vector<way> possible_back;
    for (auto const &w : ways) {
      if (std::find(visited.begin(), visited.end(), w.id_) == visited.end()) {
        if (w.back_ == front) {
          possible_front.push_back(w);
        }
        if (w.front_ == back) {
          possible_back.push_back(w);
        }
      }
    }
    if (possible_back.empty() && possible_front.empty()) {
      for (auto const &w : ways) {
        if (std::find(visited.begin(), visited.end(), w.id_) == visited.end()) {
          segments.push_back({w});
          visited.push_back(w.id_);
          break;
        }
      }
    }
    if (possible_back.size() == 1) {
      currentway.push_back(possible_back.front());
      visited.push_back(possible_back.front().id_);
    }
    if (possible_front.size() == 1) {
      currentway.insert(currentway.begin(), possible_front.front());
      visited.push_back(possible_front.front().id_);
    }
  }
  for (auto const &s : segments) {
    for (auto const &w : s) {
      result.push_back(w.id_);
    }
  }
  return result;
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
