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
      services_.emplace_back(new hrd_service(*s.first));
      s.second = services_.back().get();
    }
    return s.second;
  }

  void create_graph() {
    std::vector<std::unique_ptr<node>> nodes;
    std::map<hrd_service*, node*> service_to_node;

    int layer = 0;
    std::vector<std::map<node*, std::set<node*>>> referenced_by(1);
    std::vector<std::vector<node*>> layer_nodes(1);
    for (auto const& rule_entry : rules_) {
      for (auto const& rule : rule_entry.second) {
        for (auto const& comb : rule->service_combinations()) {
          auto const& s1 = std::get<0>(comb);
          auto const& s2 = std::get<1>(comb);

          auto s1_node =
              motis::loader::get_or_create(service_to_node, s1, [&]() {
                nodes.emplace_back(new service_node(s1));
                return nodes.back().get();
              });
          auto s2_node =
              motis::loader::get_or_create(service_to_node, s2, [&]() {
                nodes.emplace_back(new service_node(s2));
                return nodes.back().get();
              });

          nodes.emplace_back(new rule_node(
              reinterpret_cast<service_node*>(s1_node),
              reinterpret_cast<service_node*>(s2_node), std::get<2>(comb)));
          auto rule_node = nodes.back().get();
          referenced_by[layer][s1_node].insert(rule_node);
          referenced_by[layer][s2_node].insert(rule_node);
          layer_nodes[layer].push_back(rule_node);
        }
      }
    }

    while (!referenced_by[layer].empty()) {
      auto next_layer = layer + 1;

      referenced_by.resize(next_layer);
      layer_nodes.resize(next_layer);

      std::set<std::pair<node*, node*>> node_combinations;

      for (auto const& node : layer_nodes[layer]) {
        for (auto const& neighbor : node->neighbors()) {
          for (auto const& ext_neighbor : referenced_by[layer][neighbor]) {
            if (ext_neighbor == node) {
              continue;
            }

            if (node_combinations.find(std::make_pair(ext_neighbor, node)) !=
                end(node_combinations)) {
              continue;
            }
            node_combinations.emplace(node, ext_neighbor);

            layer_node new_node(ext_neighbor, node);
            if (new_node.traffic_days().none()) {
              continue;
            }

            nodes.emplace_back(new layer_node(new_node));
            auto persistent = nodes.back().get();

            referenced_by[next_layer][node].insert(persistent);
            referenced_by[next_layer][ext_neighbor].insert(persistent);
            layer_nodes[next_layer].push_back(persistent);
          }
        }
      }
      ++layer;
    }
  }

  rules rules_;
  std::vector<std::unique_ptr<hrd_service>> services_;
};

}  // hrd
}  // loader
}  // motis
