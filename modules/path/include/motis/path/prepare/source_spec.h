#pragma once

#include <set>
#include <vector>

namespace motis {
namespace path {

struct source_spec {
  enum class category { UNKNOWN, RAILWAY, BUS };

  enum class type { RELATION, OSRM_ROUTE, STUB_ROUTE, RAIL_ROUTE };

  source_spec() = default;
  source_spec(int64_t id, category c, type t)
      : id_(id), category_(c), type_(t) {}

  std::string type_str() {
    switch(type_) {
      case type::RELATION: return "RELATION";
      case type::OSRM_ROUTE: return "OSRM_ROUTE";
      case type::STUB_ROUTE: return "STUB_ROUTE";
      case type::RAIL_ROUTE: return "RAIL_ROUTE";
      default: return "INVALID";
    }
  }

  int64_t id_;
  category category_;
  type type_;
};

inline bool operator==(source_spec::category const t, int const category) {
  switch (t) {
    case source_spec::category::RAILWAY: return category < 6;
    default: return category >= 6;
  };
}

inline std::vector<std::pair<source_spec::category, std::vector<uint32_t>>>
category_groups(std::set<int> const& motis_categories) {
  // xx this is a bit stubish
  using group = std::pair<source_spec::category, std::vector<uint32_t>>;
  auto railway = group{source_spec::category::RAILWAY, {}};
  auto unknown = group{source_spec::category::UNKNOWN, {}};
  auto bus = group{source_spec::category::BUS, {}};

  for (auto const& category : motis_categories) {
    if (category < 6) {
      railway.second.push_back(category);
    } else if (category == 8) {
      bus.second.push_back(category);
    } else {
      unknown.second.push_back(category);
    }
  }

  std::vector<group> result;
  if (!railway.second.empty()) {
    result.emplace_back(std::move(railway));
  }
  if (!unknown.second.empty()) {
    result.emplace_back(std::move(unknown));
  }
  if (!bus.second.empty()) {
    result.emplace_back(std::move(bus));
  }
  return result;
}

}  // namespace path
}  // namespace motis
