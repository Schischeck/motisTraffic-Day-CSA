#pragma once

#include <memory>
#include <map>
#include <set>
#include <vector>

#include "motis/loader/util.h"

#include "motis/loader/parsers/hrd/service/hrd_service.h"
#include "motis/loader/parsers/hrd/service_rules/rule.h"
#include "motis/loader/parsers/hrd/service_rules/rules_graph.h"
#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

struct rule_service_builder {
  rule_service_builder() = default;
  rule_service_builder(rules rules) : rules_(std::move(rules)) {}

  bool add_service(hrd_service const& s);
  void build();

  std::vector<std::unique_ptr<hrd_service>> origin_services_;
  std::vector<rule_service> rule_services_;

private:
  rules rules_;
};

}  // hrd
}  // loader
}  // motis
