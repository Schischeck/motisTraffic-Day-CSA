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

void rule_node::resolve_services(
    bitfield const& upper_traffic_days,
    std::set<service_resolvent>& s_resolvents,
    std::vector<service_rule_resolvent>& sr_resolvents) {
  if (traffic_days_.any()) {
    auto active_traffic_days = traffic_days_ & upper_traffic_days;
    traffic_days_ &= ~active_traffic_days;
    sr_resolvents.emplace_back(
        rule_,  //
        resolve(active_traffic_days, s1_->service_, s_resolvents),
        resolve(active_traffic_days, s2_->service_, s_resolvents));
  }
}

service_node::service_node(hrd_service* s) : service_(s) {}

rule_node::rule_node(service_node* s1, service_node* s2,
                     resolved_rule_info rule_info)
    : s1_(s1),
      s2_(s2),
      rule_(rule_info),
      traffic_days_(s1->service_->traffic_days_ & s2->service_->traffic_days_ &
                    rule_info.traffic_days) {}

std::pair<std::set<rule_node*>, bitfield> rule_node::max_component() {
  std::pair<std::set<rule_node*>, bitfield> max;
  auto& component_nodes = max.first;
  auto& component_traffic_days = max.second;

  rule_node* current = nullptr;
  std::set<rule_node*> queue = {this};
  component_traffic_days = create_uniform_bitfield<BIT_COUNT>('1');
  while (!queue.empty()) {
    auto first_element = queue.begin();
    current = *first_element;
    queue.erase(first_element);

    auto next_traffic_days = component_traffic_days & current->traffic_days_;
    if (next_traffic_days.none()) {
      continue;
    }
    component_traffic_days = next_traffic_days;
    component_nodes.insert(current);

    for (auto const& link_node : {current->s1_, current->s2_}) {
      for (auto const& related_node : link_node->rule_nodes_) {
        if (related_node != current &&
            component_nodes.find(related_node) != end(component_nodes)) {
          queue.insert(related_node);
        }
      }
    }
  }

  return max;
}

}  // hrd
}  // loader
}  // motis
