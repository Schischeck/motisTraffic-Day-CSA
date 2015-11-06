#pragma once

#include <set>
#include <memory>
#include <map>
#include <vector>
#include <functional>

#include "motis/schedule-format/RuleService_generated.h"

#include "motis/loader/util.h"
#include "motis/loader/hrd/model/hrd_service.h"
#include "motis/loader/hrd/model/service_rule.h"
#include "motis/loader/hrd/model/rule_service.h"

namespace motis {
namespace loader {
namespace hrd {

struct rule_service_builder {
  rule_service_builder() = default;
  rule_service_builder(service_rules rs) : rules_(std::move(rs)) {}

  bool add_service(hrd_service const&);
  void resolve_rule_services();
  typedef std::function<flatbuffers::Offset<Service>(
      hrd_service const&, flatbuffers::FlatBufferBuilder&)> service_builder_fun;
  void create_rule_services(service_builder_fun,
                            flatbuffers::FlatBufferBuilder&);

  std::vector<std::unique_ptr<hrd_service>> origin_services_;
  std::vector<rule_service> rule_services_;
  std::vector<flatbuffers::Offset<RuleService>> fbs_rule_services;

private:
  service_rules rules_;
};

}  // hrd
}  // loader
}  // motis
