#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "geo/latlng.h"

#include "motis/path/prepare/source_spec.h"

namespace motis {
namespace path {

struct node {
  node() = default;
  explicit node(int64_t id) : id_(id), resolved_(false) {}

  int64_t id_ = 0;
  bool resolved_ = false;
  geo::latlng pos_;
};

struct way {
  explicit way(int64_t id) : id_(id), resolved_(false) {}

  node const& head() const { return nodes_.back(); }
  node const& tail() const { return nodes_.front(); }

  int64_t id_ = 0;
  bool resolved_ = false;
  std::vector<node> nodes_;
};

struct relation {
  relation(source_spec source, std::vector<way*> ways)
      : source_(source), ways_(std::move(ways)) {}

  source_spec source_;
  std::vector<way*> ways_;
};

struct parsed_relations {
  std::vector<relation> relations_;
  std::vector<std::unique_ptr<way>> way_mem_;
};

parsed_relations parse_relations(std::string const& osm_file);

}  // namespace path
}  // namespace motis
