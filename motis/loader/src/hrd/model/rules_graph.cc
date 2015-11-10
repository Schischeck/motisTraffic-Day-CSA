#include "motis/loader/hrd/model/rules_graph.h"

namespace motis {
namespace loader {
namespace hrd {

service_node::service_node(hrd_service* s) : service_(s) {}

void service_node::services(std::set<hrd_service*>& acc) const {
  acc.insert(service_);
};

void service_node::resolve_services(bitfield const&,
                                    std::set<service_resolvent>&,
                                    std::set<service_rule_resolvent>&) {}

std::array<node*, 2> service_node::children() const {
  return {{nullptr, nullptr}};
}

bitfield const& service_node::traffic_days() const {
  return service_->traffic_days_;
}

void service_node::print() const {
  printf("(%p) service_node [%s] - [%d,%.*s] - %s:%d\n", this,
         traffic_days().to_string().c_str(), service_->sections_[0].train_num,
         (int)service_->sections_[0].admin.length(),
         service_->sections_[0].admin.c_str(), service_->origin_.filename,
         service_->origin_.line_number_from);
}

rule_node::rule_node(service_node* s1, service_node* s2,
                     resolved_rule_info rule_info)
    : s1_(s1),
      s2_(s2),
      rule_(rule_info),
      traffic_days_(s1->traffic_days() & s2->traffic_days() &
                    rule_info.traffic_days) {}

void rule_node::services(std::set<hrd_service*>& acc) const {
  s1_->services(acc);
  s2_->services(acc);
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

void rule_node::resolve_services(
    bitfield const& upper_traffic_days,
    std::set<service_resolvent>& s_resolvents,
    std::set<service_rule_resolvent>& sr_resolvents) {
  if (traffic_days_.any()) {
    auto active_traffic_days = traffic_days_ & upper_traffic_days;
    traffic_days_ &= ~active_traffic_days;
    sr_resolvents.insert(service_rule_resolvent{
        rule_, resolve(active_traffic_days, s1_->service_, s_resolvents),
        resolve(active_traffic_days, s2_->service_, s_resolvents)});
  }
}

std::array<node*, 2> rule_node::children() const { return {{s1_, s2_}}; }

bitfield const& rule_node::traffic_days() const { return traffic_days_; }

void rule_node::print() const {
  printf("(%p) rule_node    [%s] - %s\n", this,
         traffic_days().to_string().c_str(), rule_.type == 0 ? "TS" : "MSS");
}

layer_node::layer_node(node* left, node* right)
    : left_(left),
      right_(right),
      traffic_days_(left->traffic_days() & right->traffic_days()) {}

void layer_node::services(std::set<hrd_service*>& acc) const {
  left_->services(acc);
  right_->services(acc);
}

void layer_node::resolve_services(bitfield const& upper_traffic_days,
                                  std::set<service_resolvent>& resolvents,
                                  std::set<service_rule_resolvent>& rules) {
  if (traffic_days_.any()) {
    auto active_traffic_days = traffic_days_ & upper_traffic_days;
    traffic_days_ &= ~active_traffic_days;
    left_->resolve_services(active_traffic_days, resolvents, rules);
    right_->resolve_services(active_traffic_days, resolvents, rules);
  }
}

std::array<node*, 2> layer_node::children() const { return {{left_, right_}}; }

bitfield const& layer_node::traffic_days() const { return traffic_days_; }

void layer_node::print() const {
  printf("(%p) layer_node   [%s]\n", this, traffic_days().to_string().c_str());
}

void rules_graph::print_nodes() const {
  // print layer nodes
  for (int layer_idx = layers_.size() - 1; layer_idx >= 0; --layer_idx) {
    printf("layer %d:\n", layer_idx);
    for (auto const* parent : layers_[layer_idx]) {
      auto const* child_1 = parent->children().at(0);
      auto const* child_2 = parent->children().at(1);
      parent->print();
      child_1->print();
      child_2->print();
    }
  }
}

}  // hrd
}  // loader
}  // motis
