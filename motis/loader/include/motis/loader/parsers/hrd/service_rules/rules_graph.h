#pragma once

#include <set>
#include <array>

#include "motis/loader/parsers/hrd/service/hrd_service.h"

namespace motis {
namespace loader {
namespace hrd {

struct resolved_service {
  resolved_service(hrd_service* origin) : service(nullptr), origin(origin) {}

  resolved_service(std::unique_ptr<hrd_service> service, hrd_service* origin)
      : service(std::move(service)), origin(origin) {}

  friend bool operator<(resolved_service const& rhs,
                        resolved_service const& lhs) {
    return rhs.origin < lhs.origin;
  }

  friend bool operator==(resolved_service const& rhs,
                         resolved_service const& lhs) {
    return rhs.origin == lhs.origin;
  }

  std::unique_ptr<hrd_service> service;
  hrd_service* origin;
};

struct node {
  virtual ~node() {}

  virtual void resolve_services(
      bitfield const& /* upper_traffic_days */,
      std::set<resolved_service>& /* resolved_services */,
      std::vector<std::tuple<resolved_rule_info, hrd_service*, hrd_service*>>&
      /* rules */){};

  virtual std::array<node*, 2> children() const = 0;
  virtual bitfield const& traffic_days() const = 0;

  std::vector<node*> parents_;
};

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

  void resolve_services(
      bitfield const& upper_traffic_days,
      std::set<resolved_service>& resolved_services,
      std::vector<std::tuple<resolved_rule_info, hrd_service*, hrd_service*>>&
          rules) override {
    rules.emplace_back(std::make_tuple(
        rule_, resolve(upper_traffic_days, s1_->service, resolved_services),
        resolve(upper_traffic_days, s2_->service, resolved_services)));
  }

  hrd_service* resolve(bitfield const& upper_traffic_days, hrd_service* origin,
                       std::set<resolved_service>& resolved_services) {
    auto resolved_it = resolved_services.find(resolved_service(origin));
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

  service_node* s1_, *s2_;
  resolved_rule_info rule_;
  bitfield traffic_days_;
};

struct layer_node : public node {
  layer_node(node* left, node* right)
      : left_(left),
        right_(right),
        traffic_days_(left->traffic_days() & right->traffic_days()) {}

  virtual ~layer_node() {}

  void resolve_services(
      bitfield const& upper_traffic_days,
      std::set<resolved_service>& resolved_services,
      std::vector<std::tuple<resolved_rule_info, hrd_service*, hrd_service*>>&
          rules) override {
    auto active_traffic_days = traffic_days_ & upper_traffic_days;
    traffic_days_ &= ~active_traffic_days;

    left_->resolve_services(active_traffic_days, resolved_services, rules);
    right_->resolve_services(active_traffic_days, resolved_services, rules);
  }

  std::array<node*, 2> children() const override { return {{left_, right_}}; }

  virtual bitfield const& traffic_days() const override {
    return traffic_days_;
  }

  node* left_, *right_;
  bitfield traffic_days_;
};

}  // hrd
}  // loader
}  // motis
