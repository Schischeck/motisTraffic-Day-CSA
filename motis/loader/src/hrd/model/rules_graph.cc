#include "motis/loader/hrd/model/rules_graph.h"

#include <numeric>

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

service_node::service_node(hrd_service* s)
    : node(std::vector<node*>(), std::set<hrd_service*>(),
           std::set<rule_node_info*>()),
      service_(s) {}

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
    : node({s1, s2}, {s1->service_, s2->service_}, {&rule_}),
      s1_(s1),
      s2_(s2),
      rule_(
          {s1->service_, s2->service_, rule_info,
           s1->traffic_days() & s2->traffic_days() & rule_info.traffic_days}) {}

bitfield const& rule_node::traffic_days() const { return rule_.traffic_days_; }

void rule_node::print() const {
  printf("(%p) rule_node    [%s] - %s\n", this,
         traffic_days().to_string().c_str(),
         rule_.rule_.type == 0 ? "TS" : "MSS");
}

std::set<hrd_service*> joined_services(std::vector<node*> const& children) {
  std::set<hrd_service*> services;
  for (auto const& c : children) {
    services.insert(begin(c->services_), end(c->services_));
  }
  return services;
}

std::set<rule_node_info*> joined_rule_node_infos(
    std::vector<node*> const& children) {
  std::set<rule_node_info*> rules;
  for (auto const& c : children) {
    rules.insert(begin(c->rules_), end(c->rules_));
  }
  return rules;
}

layer_node::layer_node(std::vector<node*> const& children)
    : node(children, joined_services(children),
           joined_rule_node_infos(children)),
      traffic_days_(std::accumulate(begin(children), end(children),
                                    create_uniform_bitfield<BIT_COUNT>('1'),
                                    [](bitfield const& acc, node * n) {
                                      return acc & n->traffic_days();
                                    })) {}

bitfield const& layer_node::traffic_days() const { return traffic_days_; }

void layer_node::print() const {
  printf("(%p) layer_node   [%s]\n", this, traffic_days().to_string().c_str());
}

void rules_graph::print_nodes() const {
  // print layer nodes
  for (int layer_idx = layers_.size() - 1; layer_idx >= 0; --layer_idx) {
    printf("layer %d:\n", layer_idx);
    for (auto const* parent : layers_[layer_idx]) {
      auto const* child_1 = parent->children_.at(0);
      auto const* child_2 = parent->children_.at(1);
      parent->print();
      child_1->print();
      child_2->print();
    }
  }
}

}  // hrd
}  // loader
}  // motis
