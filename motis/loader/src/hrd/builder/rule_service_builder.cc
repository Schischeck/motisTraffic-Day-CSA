#include "motis/loader/hrd/builder/rule_service_builder.h"

#include <algorithm>
#include <bitset>
#include <iterator>
#include <tuple>
#include <utility>

#include "parser/util.h"

#include "motis/core/common/logging.h"

#include "motis/loader/hrd/model/rules_graph.h"
#include "motis/loader/hrd/graph_visualization.h"

namespace motis {
namespace loader {
namespace hrd {

void print_tree(node const* root) {
  root->print();
  for (auto const* child : root->children_) {
    if (child) {
      print_tree(child);
    }
  }
}

using namespace logging;
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

    auto const& rule = std::get<2>(comb);
    rg.nodes_.emplace_back(
        new rule_node(reinterpret_cast<service_node*>(s1_node),
                      reinterpret_cast<service_node*>(s2_node), rule));

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

bool add_layer_node(node* n1, node* n2,
                    std::set<std::pair<node*, node*>>& combinations,
                    rules_graph& rg) {
  if (n1 == n2 ||
      combinations.find(std::make_pair(n2, n1)) != end(combinations)) {
    return false;
  }
  combinations.emplace(n1, n2);

  layer_node candidate({n1, n2});
  if (candidate.traffic_days().none()) {
    return false;
  }

  if (n1->services_ == n2->services_) {
    return false;
  }

  rg.nodes_.emplace_back(new layer_node(candidate));
  return true;
}

void build_remaining_layers(rules_graph& rg) {
  int current_layer_idx = 0;
  int next_layer_idx = 1;

  while (!rg.layers_[current_layer_idx].empty()) {
    rg.layers_.resize(next_layer_idx + 1);

    std::set<std::pair<node*, node*>> combinations;
    auto count = 0;
    for (auto const& current_parent : rg.layers_[current_layer_idx]) {
      for (auto const& child : current_parent->children_) {
        for (auto const related_parent : child->parents_) {
          if (add_layer_node(current_parent, related_parent, combinations,
                             rg)) {
            auto ln = rg.nodes_.back().get();
            current_parent->parents_.push_back(ln);
            related_parent->parents_.push_back(ln);
            rg.layers_[next_layer_idx].push_back(ln);
          }
          ++count;
        }
      }
    }

    auto const current_node_count = rg.layers_[current_layer_idx].size();
    auto const next_node_count = rg.layers_[next_layer_idx].size();
    if (current_node_count == next_node_count) {
      LOG(warn) << "found potential cycle at layer " << next_layer_idx << ": "
                << "current_node_count=" << current_node_count << ", "
                << "next_node_count=" << next_node_count;
    } else if (current_node_count < next_node_count) {
      LOG(warn) << "suspicious node count increase at layer " << next_layer_idx
                << ": "
                << "current_node_count=" << current_node_count << ", "
                << "next_node_count=" << next_node_count;
    } else {
      LOG(debug) << "#iter=" << count << ": "
                 << "layer_idx=" << next_layer_idx << ": "
                 << "current_node_count=" << current_node_count << ", "
                 << "next_node_count=" << next_node_count;
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
  scoped_timer("resolve service rules");

  rules_graph rg;
  build_graph(rules_, rg);
  //  rg.print_nodes();

  std::for_each(
      rg.layers_.rbegin(), rg.layers_.rend(), [&](std::vector<node*>& layer) {
        for (auto const& l : layer) {
          if (!l->parents_.empty() &&
              std::all_of(begin(l->parents_), end(l->parents_),
                          [&l](node const* p) {
                            return p->traffic_days() == l->traffic_days();
                          })) {
            continue;
          }

          std::set<service_resolvent> s_resolvents;
          std::vector<service_rule_resolvent> sr_resolvents;
          for (auto const& r : l->rules_) {
            r->resolve_services(l->traffic_days(), s_resolvents, sr_resolvents);
          }
          if (!sr_resolvents.empty()) {
            rule_services_.emplace_back(std::move(sr_resolvents),
                                        std::move(s_resolvents));
          }
        }
      });
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
        services.at(r.s1), services.at(r.s2), r.rule_info.eva_num_1,
        r.rule_info.eva_num_2));
  }
  fbs_rule_services.push_back(
      CreateRuleService(fbb, fbb.CreateVector(fbb_rules)));
}

void rule_service_builder::create_rule_services(service_builder_fun sbf,
                                                FlatBufferBuilder& fbb) {
  scoped_timer("create rule and remaining services");
  LOG(info) << "#remaining services: " << origin_services_.size();
  for (auto const& s : origin_services_) {
    if (s->traffic_days_.any()) {
      sbf(std::cref(*s.get()), std::ref(fbb));
    }
  }
  LOG(info) << "#rule services: " << rule_services_.size();
  for (auto const& rs : rule_services_) {
    create_rule_service(rs, sbf, fbs_rule_services, fbb);
  }
}

}  // hrd
}  // loader
}  // motis
