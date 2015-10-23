#pragma once

#include <set>
#include <array>

#include "motis/loader/bitfield.h"
#include "motis/loader/parsers/hrd/service_rules/rule_service.h"

namespace motis {
namespace loader {
namespace hrd {

struct node {
  virtual ~node() {}

  virtual void resolve_services(bitfield const&, std::set<service_resolvent>&,
                                std::set<service_rule_resolvent>&) = 0;

  virtual std::array<node*, 2> children() const = 0;
  virtual bitfield const& traffic_days() const = 0;

  std::vector<node*> parents_;
};

struct service_node : node {
  service_node(hrd_service*);

  ~service_node() {}

  void resolve_services(bitfield const&, std::set<service_resolvent>&,
                        std::set<service_rule_resolvent>&) override;
  std::array<node*, 2> children() const override;
  bitfield const& traffic_days() const override;

  hrd_service* service_;
};

struct rule_node : node {
  rule_node(service_node*, service_node*, resolved_rule_info);

  ~rule_node() {}

  void resolve_services(bitfield const&, std::set<service_resolvent>&,
                        std::set<service_rule_resolvent>&) override;
  std::array<node*, 2> children() const override;
  bitfield const& traffic_days() const override;

  service_node* s1_;
  service_node* s2_;
  resolved_rule_info rule_;
  bitfield traffic_days_;
};

struct layer_node : public node {
  layer_node(node*, node*);

  ~layer_node() {}

  void resolve_services(bitfield const&, std::set<service_resolvent>&,
                        std::set<service_rule_resolvent>&) override;
  std::array<node*, 2> children() const override;
  bitfield const& traffic_days() const override;

  node* left_;
  node* right_;
  bitfield traffic_days_;
};

struct rules_graph {
  void print_nodes();

  std::vector<std::unique_ptr<node>> nodes_;
  std::vector<std::vector<node*>> layers_;
};

}  // hrd
}  // loader
}  // motis
