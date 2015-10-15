#pragma once

#include <memory>
#include <map>
#include <set>
#include <vector>

#include "motis/loader/util.h"

#include "motis/loader/parsers/hrd/service/hrd_service.h"
#include "motis/loader/parsers/hrd/service_rules/rule.h"
#include "motis/loader/parsers/hrd/service_rules/rules_graph.h"
#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

struct service_rules {
  service_rules(rules rules) : rules_(std::move(rules)) {}

  bool add_service(hrd_service const& s) {
    std::set<rule*> rules;
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
        try_apply_rules(copied_service, it->second);
      }
    }

    return copied_service.second != nullptr;
  }

  void try_apply_rules(std::pair<hrd_service const*, hrd_service*>& s,
                       std::vector<std::shared_ptr<rule>>& rules) {
    for (auto& r : rules) {
      int info = r->applies(*s.first);
      if (info) {
        r->add(get_or_create(s), info);
      }
    }
  }

  hrd_service* get_or_create(std::pair<hrd_service const*, hrd_service*>& s) {
    if (s.second == nullptr) {
      origin_services_.emplace_back(new hrd_service(*s.first));
      s.second = origin_services_.back().get();
    }
    return s.second;
  }

  void create_graph() {
    std::vector<std::vector<node*>> layers(1);
    std::map<hrd_service*, node*> service_to_node;
    std::set<rule*> considered_rules;
    for (auto const& rule_entry : rules_) {
      for (auto const& rule : rule_entry.second) {
        if (considered_rules.find(rule.get()) != end(considered_rules)) {
          continue;
        }
        considered_rules.insert(rule.get());

        for (auto const& comb : rule->service_combinations()) {
          auto const& s1 = std::get<0>(comb);
          auto const& s2 = std::get<1>(comb);

          auto s1_node =
              motis::loader::get_or_create(service_to_node, s1, [&]() {
                nodes_.emplace_back(new service_node(s1));
                return nodes_.back().get();
              });
          auto s2_node =
              motis::loader::get_or_create(service_to_node, s2, [&]() {
                nodes_.emplace_back(new service_node(s2));
                return nodes_.back().get();
              });

          nodes_.emplace_back(new rule_node(
              reinterpret_cast<service_node*>(s1_node),
              reinterpret_cast<service_node*>(s2_node), std::get<2>(comb)));
          auto rule_node = nodes_.back().get();
          s1_node->parents_.push_back(rule_node);
          s2_node->parents_.push_back(rule_node);
          layers[0].push_back(rule_node);
          printf("create s1_node from   [%p]\n", s1);
          printf("create s2_node from   [%p]\n", s2);
          printf("create rule_node from [%p]\n", rule.get());

          printf("[%p]-(%p)-[%p]\n", s1_node, rule_node, s2_node);

          printf("s1_node:    traffic_days: [%s]\n",
                 s1_node->traffic_days().to_string().c_str());
          printf("s2_node:    traffic_days: [%s]\n",
                 s2_node->traffic_days().to_string().c_str());
          printf("rule_node:  traffic_days: [%s]\n",
                 rule_node->traffic_days().to_string().c_str());
        }
      }
    }

    int prev_layer_idx = 0;
    int next_layer_idx = 1;
    while (!layers[prev_layer_idx].empty()) {
      layers.resize(next_layer_idx + 1);

      printf("(prev_layer_idx, next_layer_idx) = (%d, %d)\n", prev_layer_idx,
             next_layer_idx);

      std::set<std::pair<node*, node*>> considered_combinations;
      for (auto const& node : layers[prev_layer_idx]) {
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
            nodes_.emplace_back(new layer_node(candidate));

            auto parent = nodes_.back().get();
            related_node->parents_.push_back(parent);
            node->parents_.push_back(parent);
            layers[next_layer_idx].push_back(parent);
            printf("[%p]-(%p)-[%p]\n", node, parent, related_node);
            printf("layer_node traffic_days: [%s]\n",
                   parent->traffic_days().to_string().c_str());
          }
        }
      }
      ++prev_layer_idx;
      ++next_layer_idx;
    }

    std::for_each(
        layers.rbegin(), layers.rend(), [&](std::vector<node*>& layer) {
          layer.erase(
              std::remove_if(begin(layer), end(layer), [](node const* n) {
                return n->traffic_days().none();
              }), end(layer));
          std::for_each(begin(layer), end(layer), [&](node* n) {
            std::vector<rule_service> rule_service_seq;
            n->resolve_services(n->traffic_days(), resolved_services_,
                                rule_service_seq);
            if (!rule_service_seq.empty()) {
              rule_services_.push_back(rule_service_seq);
            }
          });
        });

    origin_services_.erase(
        std::remove_if(begin(origin_services_), end(origin_services_),
                       [](std::unique_ptr<hrd_service> const& service_ptr) {
                         return service_ptr.get()->traffic_days_.none();
                       }),
        end(origin_services_));
  }

  rules rules_;
  std::vector<std::unique_ptr<hrd_service>> origin_services_;
  std::set<resolved_service> resolved_services_;
  std::vector<std::unique_ptr<node>> nodes_;
  std::vector<std::vector<rule_service>> rule_services_;
};

}  // hrd
}  // loader
}  // motis
