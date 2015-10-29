#include "motis/loader/builders/hrd/rule_service_builder.h"

#include <bits/shared_ptr_base.h>
#include <algorithm>
#include <bitset>
#include <iterator>
#include <tuple>
#include <utility>

#include "motis/loader/model/hrd/rules_graph.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace flatbuffers;

hrd_service* get_or_create(
    std::vector<std::unique_ptr<hrd_service>>& origin_services,
    std::pair<hrd_service const*, hrd_service*>& s) {
  if (s.second == nullptr) {
    origin_services.emplace_back(new hrd_service(*s.first));
    s.second = origin_services.back().get();
  }
  return s.second;
}

void try_apply_rules(std::vector<std::unique_ptr<hrd_service>>& origin_services,
                     std::pair<hrd_service const*, hrd_service*>& s,
                     std::vector<std::shared_ptr<service_rule>>& rules) {
  for (auto& r : rules) {
    int info = r->applies(*s.first);
    if (info) {
      r->add(get_or_create(origin_services, s), info);
    }
  }
}

bool rule_service_builder::add_service(hrd_service const& s) {
  std::set<service_rule*> rules;
  for (auto const& entry : rules_) {
    for (auto const& rule : entry.second) {
      rules.insert(rule.get());
    }
  }

  std::pair<hrd_service const*, hrd_service*> copied_service;
  copied_service.first = &s;  // the original service
  copied_service.second = nullptr;  // pointer to the copy if we need it

  for (auto const& id : s.get_ids()) {
    auto it = rules_.find(id);
    if (it != end(rules_)) {
      try_apply_rules(origin_services_, copied_service, it->second);
    }
  }

  return copied_service.second != nullptr;
}

void create_rule_and_service_nodes(
    service_rule* r, rules_graph& rg,
    std::map<hrd_service*, node*>& service_to_node) {
  for (auto const& comb : r->service_combinations()) {
    auto const& s1 = std::get<0>(comb);
    auto const& s2 = std::get<1>(comb);

    auto s1_node = motis::loader::get_or_create(service_to_node, s1, [&]() {
      rg.nodes_.emplace_back(new service_node(s1));
      return rg.nodes_.back().get();
    });
    auto s2_node = motis::loader::get_or_create(service_to_node, s2, [&]() {
      rg.nodes_.emplace_back(new service_node(s2));
      return rg.nodes_.back().get();
    });

    rg.nodes_.emplace_back(new rule_node(
        reinterpret_cast<service_node*>(s1_node),
        reinterpret_cast<service_node*>(s2_node), std::get<2>(comb)));
    auto rn = rg.nodes_.back().get();
    s1_node->parents_.push_back(rn);
    s2_node->parents_.push_back(rn);
    rg.layers_[0].push_back(rn);
  }
}

void build_first_layer(service_rules const& rules, rules_graph& rg) {
  std::set<service_rule*> considered_rules;
  std::map<hrd_service*, node*> service_to_node;

  rg.layers_.resize(1);
  for (auto const& rule_entry : rules) {
    for (auto const& sr : rule_entry.second) {
      if (considered_rules.find(sr.get()) == end(considered_rules)) {
        considered_rules.insert(sr.get());
        create_rule_and_service_nodes(sr.get(), rg, service_to_node);
      }
    }
  }
}

void build_remaining_layers(rules_graph& rg) {
  int current_layer_idx = 0;
  int next_layer_idx = 1;

  while (!rg.layers_[current_layer_idx].empty()) {
    rg.layers_.resize(next_layer_idx + 1);

    std::set<std::pair<node*, node*>> considered_combinations;
    for (auto const& node : rg.layers_[current_layer_idx]) {
      for (auto const& child : node->children()) {
        for (auto const& related_node : child->parents_) {
          if (related_node == node) {
            continue;
          }
          if (considered_combinations.find(std::make_pair(
                  related_node, node)) != end(considered_combinations)) {
            continue;
          }
          considered_combinations.emplace(node, related_node);

          layer_node candidate(node, related_node);
          if (candidate.traffic_days().none()) {
            continue;
          }
          rg.nodes_.emplace_back(new layer_node(candidate));

          auto parent = rg.nodes_.back().get();
          related_node->parents_.push_back(parent);
          node->parents_.push_back(parent);
          rg.layers_[next_layer_idx].push_back(parent);
        }
      }
    }
    ++current_layer_idx;
    ++next_layer_idx;
  }
  rg.layers_.pop_back();
}

void build_graph(service_rules const& rules, rules_graph& rg) {
  build_first_layer(rules, rg);
  build_remaining_layers(rg);
}

void rule_service_builder::resolve_rule_services() {
  rules_graph rg;
  build_graph(rules_, rg);

  // iterate all layers beginning with the top level layer
  std::for_each(
      rg.layers_.rbegin(), rg.layers_.rend(), [&](std::vector<node*>& layer) {
        // remove all layer nodes that does not have any traffic day left
        layer.erase(std::remove_if(begin(layer), end(layer), [](node const* n) {
          return n->traffic_days().none();
        }), end(layer));

        // explore each node recursively and create/collect rule services
        std::for_each(begin(layer), end(layer), [&](node* n) {
          std::set<service_resolvent> s_resolvents;
          std::set<service_rule_resolvent> r_resolvents;
          n->resolve_services(n->traffic_days(), s_resolvents, r_resolvents);
          if (!r_resolvents.empty()) {
            rule_services_.emplace_back(std::move(r_resolvents),
                                        std::move(s_resolvents));
          }
        });
      });

  // remove all remaining services that does not have any traffic day left
  origin_services_.erase(
      std::remove_if(begin(origin_services_), end(origin_services_),
                     [](std::unique_ptr<hrd_service> const& service_ptr) {
                       return service_ptr.get()->traffic_days_.none();
                     }),
      end(origin_services_));
}

void create_rule_service(
    rule_service const& rs, rule_service_builder::service_builder_fun sbf,
    std::vector<flatbuffers::Offset<RuleService>>& fbs_rule_services,
    FlatBufferBuilder& fbb) {

  std::map<hrd_service const*, Offset<Service>> services;
  for (auto const& s : rs.services) {
    auto const* service = s.service.get();
    services[service] = sbf(std::cref(*service), std::ref(fbb));
  }

  std::vector<Offset<Rule>> fbb_rules;
  for (auto const& r : rs.rules) {

    fbb_rules.push_back(CreateRule(
        fbb, r.rule_info.type == 0 ? RuleType_THROUGH : RuleType_MERGE_SPLIT,
        services.find(r.s1)->second, services.find(r.s2)->second,
        r.rule_info.eva_num_1, r.rule_info.eva_num_2));
  }
  fbs_rule_services.push_back(
      CreateRuleService(fbb, fbb.CreateVector(fbb_rules)));
}

void rule_service_builder::create_rule_services(service_builder_fun sbf,
                                                FlatBufferBuilder& fbb) {
  for (auto const& s : origin_services_) {
    sbf(std::cref(*s.get()), std::ref(fbb));
  }
  for (auto const& rs : rule_services_) {
    create_rule_service(rs, sbf, fbs_rule_services, fbb);
  }
}

}  // hrd
}  // loader
}  // motis
