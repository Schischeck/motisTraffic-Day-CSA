#pragma once

#include <set>
#include <memory>
#include <map>
#include <vector>

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/util.h"
#include "motis/loader/model/hrd/hrd_service.h"
#include "motis/loader/model/hrd/service_rule.h"
#include "motis/loader/model/hrd/rule_service.h"
#include "motis/loader/builders/hrd/service_builder.h"

namespace motis {
namespace loader {
namespace hrd {

struct rule_service_builder {
  rule_service_builder() = default;
  rule_service_builder(service_rules rs) : rules_(std::move(rs)) {}

  bool add_service(hrd_service const&);
  void resolve();
  void build(service_builder&);

  std::vector<std::unique_ptr<hrd_service>> origin_services_;
  std::vector<rule_service> rule_services_;
  std::vector<flatbuffers::Offset<RuleService>> fbs_rule_services;

private:
  service_rules rules_;
};

}  // hrd
}  // loader
}  // motis
