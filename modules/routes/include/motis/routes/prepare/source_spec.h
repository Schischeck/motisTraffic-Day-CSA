#pragma once

#include <set>
#include <vector>

namespace motis {
namespace routes {

struct source_spec {
  enum class category { UNKNOWN, RAILWAY };
  enum class type { RELATION, AIRLINE, POLYLINE};

  source_spec() = default;
  source_spec(int64_t id, category c, type t) : id_(id), category_(c), type_(t) {}

  int64_t id_;
  category category_;
  type type_;
  bool station_ = false;
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

  for (auto const& category : motis_categories) {
    if (category < 6) {
      railway.second.push_back(category);
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
  return result;
}

}  // namespace routes
}  // namespace motis
