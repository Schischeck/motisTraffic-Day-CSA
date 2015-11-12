#pragma once

#include <array>
#include <set>
#include <vector>

#include "motis/loader/bitfield.h"
#include "motis/loader/hrd/model/rule_service.h"

namespace motis {
namespace loader {
namespace hrd {

struct rule_node_info {
  void resolve_services(bitfield const& upper_traffic_days,
                        std::set<service_resolvent>& s_resolvents,
                        std::vector<service_rule_resolvent>& sr_resolvents);

  hrd_service* s1_;
  hrd_service* s2_;
  resolved_rule_info rule_;
  bitfield traffic_days_;
};

struct node {
  node(std::set<hrd_service*> services, std::set<rule_node_info*> rules)
      : services_(std::move(services)), rules_(std::move(rules)) {}
  virtual ~node() {}

  virtual std::array<node*, 2> children() const = 0;
  virtual bitfield const& traffic_days() const = 0;
  virtual void print() const = 0;

  std::set<hrd_service*> services_;
  std::set<rule_node_info*> rules_;
  std::vector<node*> parents_;
};

struct service_node : node {
  service_node(hrd_service*);
  ~service_node() {}

  std::array<node*, 2> children() const override;
  bitfield const& traffic_days() const override;
  void print() const override;

  hrd_service* service_;
};

struct rule_node : node {
  rule_node(service_node*, service_node*, resolved_rule_info);
  ~rule_node() {}

  std::array<node*, 2> children() const override;
  bitfield const& traffic_days() const override;
  void print() const override;

  service_node *s1_, *s2_;
  rule_node_info rule_;
};

struct layer_node : public node {
  layer_node(node*, node*);
  ~layer_node() {}

  std::array<node*, 2> children() const override;
  bitfield const& traffic_days() const override;
  void print() const override;

  node* left_;
  node* right_;
  bitfield traffic_days_;
};

struct rules_graph {
  void print_nodes() const;

  std::vector<std::unique_ptr<node>> nodes_;
  std::vector<std::vector<node*>> layers_;
};

}  // hrd
}  // loader
}  // motis
