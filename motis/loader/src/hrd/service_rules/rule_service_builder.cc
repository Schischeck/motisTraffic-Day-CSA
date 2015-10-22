#include "motis/loader/parsers/hrd/service_rules/rule_service_builder.h"

namespace motis {
namespace loader {
namespace hrd {

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
                     std::vector<std::shared_ptr<rule>>& rules) {
  for (auto& r : rules) {
    int info = r->applies(*s.first);
    if (info) {
      r->add(get_or_create(origin_services, s), info);
    }
  }
}

bool rule_service_builder::add_service(hrd_service const& s) {
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
      try_apply_rules(origin_services_, copied_service, it->second);
    }
  }

  return copied_service.second != nullptr;
}

void rule_service_builder::build() {

  rules_graph rg;
  std::vector<std::vector<node*>> layers(1);
  std::map<hrd_service*, node*> service_to_node;
  std::set<rule*> considered_rules;

  printf("\nbuild rule and service nodes ...\n");
  for (auto const& rule_entry : rules_) {
    for (auto const& r : rule_entry.second) {
      if (considered_rules.find(r.get()) != end(considered_rules)) {
        continue;
      }
      considered_rules.insert(r.get());

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
        layers[0].push_back(rn);
      }
    }
  }

  printf("\nbuild layer nodes ...\n");
  int current_layer_idx = 0;
  int next_layer_idx = 1;
  while (!layers[current_layer_idx].empty()) {
    layers.resize(next_layer_idx + 1);

    printf("(current_layer_idx, next_layer_idx) = (%d, %d)\n",
           current_layer_idx, next_layer_idx);

    std::set<std::pair<node*, node*>> considered_combinations;
    for (auto const& node : layers[current_layer_idx]) {
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
          layers[next_layer_idx].push_back(parent);
        }
      }
    }
    ++current_layer_idx;
    ++next_layer_idx;
  }
  layers.pop_back();

  printf("\nbuild rule services ...\n");

  std::for_each(layers.rbegin(), layers.rend(), [&](std::vector<node*>& layer) {
    layer.erase(std::remove_if(begin(layer), end(layer), [](node const* n) {
      return n->traffic_days().none();
    }), end(layer));

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

  origin_services_.erase(
      std::remove_if(begin(origin_services_), end(origin_services_),
                     [](std::unique_ptr<hrd_service> const& service_ptr) {
                       return service_ptr.get()->traffic_days_.none();
                     }),
      end(origin_services_));
  printf("done!\n");
}

}  // hrd
}  // loader
}  // motis
