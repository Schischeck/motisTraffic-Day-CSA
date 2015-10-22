#include "motis/loader/parsers/hrd/service_rules/rules_graph.h"

namespace motis {
namespace loader {
namespace hrd {

struct service_node : public node {
  service_node(hrd_service* service) : service(service) {}
  virtual std::array<node*, 2> children() const override {
    return {{nullptr, nullptr}};
  }
  virtual bitfield const& traffic_days() const override {
    return service->traffic_days_;
  }
  hrd_service* service;
};

struct rule_node : public node {
  rule_node(service_node* s1, service_node* s2, resolved_rule_info rule_info)
      : s1_(s1),
        s2_(s2),
        rule_(rule_info),
        traffic_days_(s1->traffic_days() & s2->traffic_days() &
                      rule_info.traffic_days) {}

  virtual ~rule_node() {}

  void resolve_services(bitfield const& upper_traffic_days,
                        std::set<service_resolvent>& resolvents,
                        std::set<service_rule_resolvent>& rules) override {
    auto active_traffic_days = traffic_days_ & upper_traffic_days;
    traffic_days_ &= ~active_traffic_days;
    rules.insert(service_rule_resolvent{
        rule_, resolve(active_traffic_days, s1_->service, resolvents),
        resolve(active_traffic_days, s2_->service, resolvents)});
  }

  hrd_service* resolve(bitfield const& upper_traffic_days, hrd_service* origin,
                       std::set<service_resolvent>& resolved_services) {
    auto resolved_it = resolved_services.find(service_resolvent(origin));
    if (resolved_it == end(resolved_services)) {
      auto resolved = make_unique<hrd_service>(*origin);
      resolved->traffic_days_ &= upper_traffic_days;
      origin->traffic_days_ &= ~upper_traffic_days;
      std::tie(resolved_it, std::ignore) =
          resolved_services.emplace(std::move(resolved), origin);
    }
    return resolved_it->service.get();
  }

  std::array<node*, 2> children() const override { return {{s1_, s2_}}; }

  virtual bitfield const& traffic_days() const override {
    return traffic_days_;
  }

  service_node* s1_;
  service_node* s2_;
  resolved_rule_info rule_;
  bitfield traffic_days_;
};

struct layer_node : public node {
  layer_node(node* left, node* right)
      : left_(left),
        right_(right),
        traffic_days_(left->traffic_days() & right->traffic_days()) {}

  virtual ~layer_node() {}

  void resolve_services(bitfield const& upper_traffic_days,
                        std::set<service_resolvent>& resolvents,
                        std::set<service_rule_resolvent>& rules) override {
    auto active_traffic_days = traffic_days_ & upper_traffic_days;
    traffic_days_ &= ~active_traffic_days;
    left_->resolve_services(active_traffic_days, resolvents, rules);
    right_->resolve_services(active_traffic_days, resolvents, rules);
  }

  std::array<node*, 2> children() const override { return {{left_, right_}}; }

  virtual bitfield const& traffic_days() const override {
    return traffic_days_;
  }

  node* left_;
  node* right_;
  bitfield traffic_days_;
};

void rules_graph::print() {
  //  printf("create s1_node from   [%p]\n", s1);
  //  printf("create s2_node from   [%p]\n", s2);
  //  printf("create rule_node from [%p]\n", r.get());

  //  printf("[%p]-(%p)-[%p]\n", s1_node, rn, s2_node);
  //  printf("mask:       traffic_days: [%s]\n",
  //         r.get()->mask_.to_string().c_str());
  //  printf("rule_node:  traffic_days: [%s]\n",
  //         rn->traffic_days().to_string().c_str());
  //  printf("s1_node:    traffic_days: [%s]\n",
  //         s1_node->traffic_days().to_string().c_str());
  //  printf("s2_node:    traffic_days: [%s]\n",
  //         s2_node->traffic_days().to_string().c_str());
  //
  //  printf("[%p]-(%p)-[%p]\n", node, parent, related_node);
  //  printf("ln_node traffic_days: [%s]\n",
  //         parent->traffic_days().to_string().c_str());
}

}  // hrd
}  // loader
}  // motis
