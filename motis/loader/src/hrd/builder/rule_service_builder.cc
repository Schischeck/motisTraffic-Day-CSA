#include "motis/loader/hrd/builder/rule_service_builder.h"

#include <algorithm>
#include <bitset>
#include <iterator>
#include <tuple>
#include <utility>

#include "parser/util.h"

#include "motis/core/common/logging.h"

#include "motis/loader/hrd/model/rules_graph.h"

namespace motis {
namespace loader {
namespace hrd {

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
    switch (rule.type) {
      case RuleType_MERGE_SPLIT: {
        s1_node->parents_.push_back(std::make_pair(rn, BOTH));
        s2_node->parents_.push_back(std::make_pair(rn, BOTH));
        break;
      }
      case RuleType_THROUGH: {
        s1_node->parents_.push_back(std::make_pair(rn, OUT));
        s2_node->parents_.push_back(std::make_pair(rn, IN));
        break;
      }
      default: { assert(false); };
    }
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

    for (auto const& current_node : rg.layers_[current_layer_idx]) {
      for (auto const& child : current_node->children()) {
        auto const entering_direction =
            std::find_if(begin(child->parents_), end(child->parents_),
                         [&](std::pair<node*, direction_type> const& parent) {
                           return parent.first == current_node;
                         })
                ->second;

        if (entering_direction == OUT) {
          LOG(debug) << "ignore OUT";
          continue;
        }

        for (auto const related_parent : child->parents_) {
          auto const leaving_node = related_parent.first;
          auto const leaving_direction = related_parent.second;

          if (leaving_node == current_node) {
            LOG(debug) << "ignore identity";
            continue;
          } else if (leaving_direction == IN) {
            LOG(debug) << "ignore IN";
            continue;
          } else if (considered_combinations.find(
                         std::make_pair(related_parent.first, current_node)) !=
                     end(considered_combinations)) {
            LOG(debug) << "ignore inverse identity pairs";
            continue;
          } else {
            considered_combinations.emplace(current_node, related_parent.first);
            layer_node candidate(current_node, related_parent.first);
            if (candidate.traffic_days().none()) {
              continue;
            } else {
              rg.nodes_.emplace_back(new layer_node(candidate));
            }
          }

          auto parent = rg.nodes_.back().get();
          if (entering_direction == IN && leaving_direction == OUT) {
            current_node->parents_.push_back(std::make_pair(parent, OUT));
            leaving_node->parents_.push_back(std::make_pair(parent, IN));
          } else if (entering_direction == BOTH && leaving_direction == OUT) {
            current_node->parents_.push_back(std::make_pair(parent, BOTH));
            leaving_node->parents_.push_back(std::make_pair(parent, IN));
          } else if (entering_direction == IN && leaving_direction == BOTH) {
            current_node->parents_.push_back(std::make_pair(parent, OUT));
            leaving_node->parents_.push_back(std::make_pair(parent, BOTH));
          } else if (entering_direction == BOTH && leaving_direction == BOTH) {
            current_node->parents_.push_back(std::make_pair(parent, BOTH));
            leaving_node->parents_.push_back(std::make_pair(parent, BOTH));
          } else {
            verify(false, "invalid edge: (%d,%d)", (int)entering_direction,
                   (int)leaving_direction);
          }
          rg.layers_[next_layer_idx].push_back(parent);
        }
      }
    }

    auto const current_node_count = rg.layers_[current_layer_idx].size();
    auto const next_node_count = rg.layers_[next_layer_idx].size();
    if (current_node_count == next_node_count) {
      LOG(warn) << "found potential cycle at layer " << next_layer_idx << ": "
                << "current_node_count=" << current_node_count << ", "
                << "next_node_count=" << next_node_count;
      break;  // TODO add handle method (Tobias Raffel)
    } else if (current_node_count < next_node_count) {
      LOG(warn) << "suspicious node count increase at layer " << next_layer_idx
                << ": "
                << "current_node_count=" << current_node_count << ", "
                << "next_node_count=" << next_node_count;
      break;  // TODO add handle method (Tobias Raffel)
    } else {
      ++current_layer_idx;
      ++next_layer_idx;
    }
  }
  rg.layers_.pop_back();
}

void build_graph(service_rules const& rules, rules_graph& rg) {
  build_first_layer(rules, rg);
  build_remaining_layers(rg);
  rg.print_nodes();
}

void rule_service_builder::resolve_rule_services() {
  scoped_timer("resolve service rules");

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
    sbf(std::cref(*s.get()), std::ref(fbb));
  }
  LOG(info) << "#rule services: " << rule_services_.size();
  for (auto const& rs : rule_services_) {
    create_rule_service(rs, sbf, fbs_rule_services, fbb);
  }
}

}  // hrd
}  // loader
}  // motis
