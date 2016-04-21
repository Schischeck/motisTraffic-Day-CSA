#pragma once

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "motis/schedule-format/RuleService_generated.h"

#include "motis/loader/hrd/builder/station_builder.h"
#include "motis/loader/hrd/model/hrd_service.h"
#include "motis/loader/hrd/model/rule_service.h"
#include "motis/loader/hrd/model/service_rule.h"
#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

struct rule_service_builder {
  rule_service_builder() = default;
  explicit rule_service_builder(service_rules rs)
      : input_rules_(std::move(rs)) {}

  bool add_service(hrd_service const&);
  void resolve_rule_services();
  typedef std::function<flatbuffers::Offset<Service>(
      hrd_service const&, flatbuffers::FlatBufferBuilder&)>
      service_builder_fun;
  void create_rule_services(service_builder_fun, station_builder&,
                            flatbuffers::FlatBufferBuilder&);

  std::vector<std::unique_ptr<hrd_service>> origin_services_;
  std::vector<rule_service> rule_services_;
  std::vector<flatbuffers::Offset<RuleService>> fbs_rule_services_;

private:
  service_rules input_rules_;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
