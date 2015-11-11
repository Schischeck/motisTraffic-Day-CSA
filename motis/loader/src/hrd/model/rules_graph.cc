#include "motis/loader/hrd/model/rules_graph.h"

namespace motis {
namespace loader {
namespace hrd {

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

void rule_node_info::resolve_services(
    bitfield const& upper_traffic_days,
    std::set<service_resolvent>& s_resolvents,
    std::vector<service_rule_resolvent>& sr_resolvents) {
  if (traffic_days_.any()) {
    auto active_traffic_days = traffic_days_ & upper_traffic_days;
    traffic_days_ &= ~active_traffic_days;
    sr_resolvents.emplace_back(rule_,  //
                               resolve(active_traffic_days, s1_, s_resolvents),
                               resolve(active_traffic_days, s2_, s_resolvents));
  }
}

service_node::service_node(hrd_service* s) : node({s}, {}), service_(s) {}

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
    : node({s1->service_, s2->service_}, {&rule_}),
      s1_(s1),
      s2_(s2),
      rule_(
          {s1->service_, s2->service_, rule_info,
           s1->traffic_days() & s2->traffic_days() & rule_info.traffic_days}) {}

std::array<node*, 2> rule_node::children() const { return {{s1_, s2_}}; }

bitfield const& rule_node::traffic_days() const { return rule_.traffic_days_; }

void rule_node::print() const {
  printf("(%p) rule_node    [%s] - %s\n", this,
         traffic_days().to_string().c_str(),
         rule_.rule_.type == 0 ? "TS" : "MSS");
}

std::set<hrd_service*> joined_services(node const* left, node const* right) {
  std::set<hrd_service*> services;
  services.insert(begin(left->services_), end(left->services_));
  services.insert(begin(right->services_), end(right->services_));
  return services;
}

std::set<rule_node_info*> joined_rule_node_infos(node const* left,
                                                 node const* right) {
  std::set<rule_node_info*> rules;
  rules.insert(begin(left->rules_), end(left->rules_));
  rules.insert(begin(right->rules_), end(right->rules_));
  return rules;
}

layer_node::layer_node(node* left, node* right)
    : node(joined_services(left, right), joined_rule_node_infos(left, right)),
      left_(left),
      right_(right),
      traffic_days_(left->traffic_days() & right->traffic_days()) {}

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
