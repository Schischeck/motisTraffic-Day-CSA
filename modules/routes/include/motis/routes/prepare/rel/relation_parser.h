#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "geo/latlng.h"

#include "motis/routes/prepare/source_spec.h"

namespace motis {
namespace routes {

struct node {
  node() = default;
  node(int64_t id) : id_(id), resolved_(false) {}

  int64_t id_;
  bool resolved_;
  geo::latlng pos_;
};

struct way {
  way(int64_t id) : id_(id), resolved_(false) {}

  node const& head() const { return nodes_.back(); }
  node const& tail() const { return nodes_.front(); }

  int64_t id_;
  bool resolved_;
  std::vector<node> nodes_;
};

struct relation {
  relation(source_spec source, std::vector<way*> ways)
      : source_(std::move(source)), ways_(std::move(ways)) {}

  source_spec source_;
  std::vector<way*> ways_;
};

struct parsed_relations {
  std::vector<relation> relations_;
  std::vector<std::unique_ptr<way>> way_mem_;
};

parsed_relations parse_relations(std::string const& osm_file);

}  // namespace routes
}  // namespace motis
