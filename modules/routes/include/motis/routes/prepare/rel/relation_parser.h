#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "motis/geo/latlng.h"

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
  enum class type { UNKNOWN, RAILWAY };

  relation(int64_t id, type t, std::vector<way*> ways)
      : id_(id), type_(t), ways_(std::move(ways)) {}

  int64_t id_;
  type type_;
  std::vector<way*> ways_;
};

struct parsed_relations {
  std::vector<relation> relations_;
  std::vector<std::unique_ptr<way>> way_mem_;
};

bool operator==(relation::type const t, int const clasz) {
  switch (t) {
    case relation::type::RAILWAY: return clasz < 6;
    default: return clasz >= 6;
  };
}

parsed_relations parse_relations(std::string const& osm_file);

}  // namespace routes
}  // namespace motis
