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
  int64_t id_;
};

struct osm_way {
  osm_way(int64_t id) : id_(id) {}

  std::vector<int64_t> nodes_;
  int64_t id_;
};

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
