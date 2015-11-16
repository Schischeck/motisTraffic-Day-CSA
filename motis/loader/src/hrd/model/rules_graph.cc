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
  std::pair<std::set<rule_node*>, bitfield> ret;
  auto& nodes = ret.first;
  auto& traffic_days = ret.second;

  rule_node* current = nullptr;
  std::set<rule_node*> q = {this};
  traffic_days = create_uniform_bitfield<BIT_COUNT>('1');
  while (!q.empty()) {
    printf("queue: ");
    for (auto const& el : q) {
      printf("%p ", el);
    }
    printf("\n");

    auto it = q.begin();
    current = *it;
    q.erase(it);

    auto next_traffic_days = traffic_days & current->traffic_days_;
    if (next_traffic_days.none()) {
      continue;
    }

    nodes.insert(current);
    traffic_days = next_traffic_days;

    for (auto const& sn : {current->s1_, current->s2_}) {
      for (auto const& rn : sn->rules_) {
        if (rn != current && nodes.find(rn) != end(nodes)) {
          q.insert(rn);
        }
      }
    }
  }

  return ret;
}

}  // hrd
}  // loader
}  // motis
