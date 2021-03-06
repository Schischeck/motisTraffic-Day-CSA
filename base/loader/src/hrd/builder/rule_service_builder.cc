#include "motis/loader/hrd/builder/rule_service_builder.h"

#include <algorithm>
#include <bitset>
#include <iterator>
#include <tuple>
#include <utility>

#include "utl/get_or_create.h"

#include "parser/util.h"

#include "motis/core/common/logging.h"
#include "motis/loader/hrd/model/rules_graph.h"
#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace logging;
using namespace flatbuffers64;

hrd_service* get_or_create(
    std::vector<std::unique_ptr<hrd_service>>& origin_services,
    std::pair<hrd_service const*, hrd_service*>& s) {
  if (s.second == nullptr) {
    origin_services.emplace_back(std::make_unique<hrd_service>(*s.first));
    s.second = origin_services.back().get();
  }
  return s.second;
}

void try_apply_rules(std::vector<std::unique_ptr<hrd_service>>& origin_services,
                     std::pair<hrd_service const*, hrd_service*>& s,
                     std::vector<std::shared_ptr<service_rule>>& rules) {
  for (auto& r : rules) {
    int info = r->applies(*s.first);
    if (info != 0) {
      if (s.first->num_repetitions_ == 0) {
        r->add(get_or_create(origin_services, s), info);
      } else {
        LOG(warn) << "suspicious service rule participant: "
                  << s.first->origin_.filename_ << " ["
                  << s.first->origin_.line_number_from_ << ","
                  << s.first->origin_.line_number_to_ << "]";
      }
    }
  }
}

bool rule_service_builder::add_service(hrd_service const& s) {
  std::pair<hrd_service const*, hrd_service*> copied_service;
  copied_service.first = &s;  // the original service
  copied_service.second = nullptr;  // pointer to the copy if we need it

  for (auto const& id : s.get_ids()) {
    auto it = input_rules_.find(id);
    if (it != end(input_rules_)) {
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

    auto s1_node = reinterpret_cast<service_node*>(
        utl::get_or_create(service_to_node, s1, [&]() {
          rg.nodes_.emplace_back(std::make_unique<service_node>(s1));
          return rg.nodes_.back().get();
        }));
    auto s2_node = reinterpret_cast<service_node*>(
        utl::get_or_create(service_to_node, s2, [&]() {
          rg.nodes_.emplace_back(std::make_unique<service_node>(s2));
          return rg.nodes_.back().get();
        }));

    auto const& rule = std::get<2>(comb);
    auto rn = reinterpret_cast<rule_node*>(  // NOLINT
        rg.nodes_
            .emplace_back(std::make_unique<rule_node>(s1_node, s2_node, rule))
            .get());

    s1_node->rule_nodes_.push_back(rn);
    s2_node->rule_nodes_.push_back(rn);
    rg.rule_nodes_.push_back(rn);
  }
}

void build_graph(service_rules const& rules, rules_graph& rg) {
  std::set<service_rule*> considered_rules;
  std::map<hrd_service*, node*> service_to_node;

  for (auto const& rule_entry : rules) {
    for (auto const& sr : rule_entry.second) {
      if (considered_rules.find(sr.get()) == end(considered_rules)) {
        considered_rules.insert(sr.get());
        create_rule_and_service_nodes(sr.get(), rg, service_to_node);
      }
    }
  }
}

void add_rule_service(
    std::pair<std::set<rule_node*>, bitfield> const& component,
    std::vector<rule_service>& rule_services) {
  std::set<service_resolvent> s_resolvents;
  std::vector<service_rule_resolvent> sr_resolvents;
  for (auto& rn : component.first) {
    rn->resolve_services(component.second, s_resolvents, sr_resolvents);
  }
  if (!sr_resolvents.empty()) {
    rule_services.emplace_back(std::move(sr_resolvents),
                               std::move(s_resolvents));
  }
}

void rule_service_builder::resolve_rule_services() {
  scoped_timer timer("resolve service rules");

  rules_graph rg;
  build_graph(input_rules_, rg);

  std::pair<std::set<rule_node*>, bitfield> component;
  for (auto const& rn : rg.rule_nodes_) {
    while (!(component = rn->max_component()).first.empty()) {
      add_rule_service(component, rule_services_);
    }
  }
}

void create_rule_service(
    rule_service const& rs,
    rule_service_builder::service_builder_fun const& sbf, station_builder& sb,
    std::vector<flatbuffers64::Offset<RuleService>>& fbs_rule_services,
    FlatBufferBuilder& fbb) {
  std::map<hrd_service const*, Offset<Service>> services;
  for (auto const& s : rs.services_) {
    auto const* service = s.service_.get();
    services[service] = sbf(std::cref(*service), true, std::ref(fbb));
  }

  std::vector<Offset<Rule>> fbb_rules;
  for (auto const& r : rs.rules_) {
    fbb_rules.push_back(
        CreateRule(fbb, static_cast<RuleType>(r.rule_info_.type_),
                   services.at(r.s1_), services.at(r.s2_),
                   sb.get_or_create_station(r.rule_info_.eva_num_1_, fbb),
                   sb.get_or_create_station(r.rule_info_.eva_num_2_, fbb)));
  }
  fbs_rule_services.push_back(
      CreateRuleService(fbb, fbb.CreateVector(fbb_rules)));
}

void rule_service_builder::create_rule_services(service_builder_fun const& sbf,
                                                station_builder& sb,
                                                FlatBufferBuilder& fbb) {
  scoped_timer timer("create rule and remaining services");
  LOG(info) << "#remaining services: " << origin_services_.size();
  for (auto const& s : origin_services_) {
    if (s->traffic_days_.any()) {
      sbf(std::cref(*s), false, std::ref(fbb));
    }
  }
  LOG(info) << "#rule services: " << rule_services_.size();
  for (auto const& rs : rule_services_) {
    create_rule_service(rs, sbf, sb, fbs_rule_services_, fbb);
  }
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
