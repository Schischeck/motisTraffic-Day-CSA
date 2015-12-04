#pragma once

#include <string>
#include <set>

#include "motis/core/schedule/schedule.h"
#include "motis/core/schedule/connection.h"

namespace motis {
namespace loader {

struct duplicate {
  bool operator<(duplicate const& d) const {
    return std::tie(route_node, connection_idx) <
           std::tie(d.route_node, d.connection_idx);
  }

  connection_info const* lc;
  node* route_node;
  uint32_t connection_idx;
};

struct duplicate_checker {
  std::set<duplicate> check(station_node const*);
  void remove_duplicates();

  schedule const& schedule_;
};

}  // namespace loader
}  // namespace motis
