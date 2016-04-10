#pragma once

#include "motis/loader/hrd/model/hrd_service.h"
#include "motis/loader/hrd/model/service_rule.h"

namespace motis {
namespace loader {
namespace hrd {

struct service_resolvent {
  service_resolvent(hrd_service* origin) : service(nullptr), origin(origin) {}

  service_resolvent(std::unique_ptr<hrd_service> service, hrd_service* origin)
      : service(std::move(service)), origin(origin) {}

  friend bool operator<(service_resolvent const& rhs,
                        service_resolvent const& lhs) {
    return rhs.origin < lhs.origin;
  }

  friend bool operator==(service_resolvent const& rhs,
                         service_resolvent const& lhs) {
    return rhs.origin == lhs.origin;
  }

  std::unique_ptr<hrd_service> service;
  hrd_service* origin;
};

struct service_rule_resolvent {
  service_rule_resolvent(resolved_rule_info rule_info, hrd_service* s1,
                         hrd_service* s2)
      : rule_info(std::move(rule_info)), s1(s1), s2(s2) {}

  resolved_rule_info rule_info;
  hrd_service* s1;
  hrd_service* s2;
};

struct rule_service {
  rule_service(std::vector<service_rule_resolvent> rules,
               std::set<service_resolvent> services)
      : rules(std::move(rules)), services(std::move(services)) {}
  std::vector<service_rule_resolvent> rules;
  std::set<service_resolvent> services;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
