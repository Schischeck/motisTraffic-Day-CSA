#pragma once

#include "motis/loader/hrd/model/hrd_service.h"
#include "motis/loader/hrd/model/service_rule.h"

namespace motis {
namespace loader {
namespace hrd {

struct service_resolvent {
  explicit service_resolvent(hrd_service* origin)
      : service_(nullptr), origin_(origin) {}

  service_resolvent(std::unique_ptr<hrd_service> service, hrd_service* origin)
      : service_(std::move(service)), origin_(origin) {}

  friend bool operator<(service_resolvent const& rhs,
                        service_resolvent const& lhs) {
    return rhs.origin_ < lhs.origin_;
  }

  friend bool operator==(service_resolvent const& rhs,
                         service_resolvent const& lhs) {
    return rhs.origin_ == lhs.origin_;
  }

  std::unique_ptr<hrd_service> service_;
  hrd_service* origin_;
};

struct service_rule_resolvent {
  service_rule_resolvent(resolved_rule_info rule_info, hrd_service* s1,
                         hrd_service* s2)
      : rule_info_(std::move(rule_info)), s1_(s1), s2_(s2) {}

  resolved_rule_info rule_info_;
  hrd_service* s1_;
  hrd_service* s2_;
};

struct rule_service {
  rule_service(std::vector<service_rule_resolvent> rules,
               std::set<service_resolvent> services)
      : rules_(std::move(rules)), services_(std::move(services)) {}
  std::vector<service_rule_resolvent> rules_;
  std::set<service_resolvent> services_;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
