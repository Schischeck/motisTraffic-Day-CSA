#pragma once

#include <set>
#include <array>

#include "motis/loader/bitfield.h"

#include "motis/loader/parsers/hrd/service/hrd_service.h"
#include "motis/loader/parsers/hrd/service_rules/rule_service.h"

namespace motis {
namespace loader {
namespace hrd {

struct node {
  virtual ~node() {}

  virtual void resolve_services(bitfield const&, std::set<service_resolvent>&,
                                std::set<service_rule_resolvent>&){};

  virtual std::array<node*, 2> children() const = 0;
  virtual bitfield const& traffic_days() const = 0;

  std::vector<node*> parents_;
};

struct rules_graph {
  void print();

  std::vector<std::unique_ptr<node>> nodes_;
};

}  // hrd
}  // loader
}  // motis
