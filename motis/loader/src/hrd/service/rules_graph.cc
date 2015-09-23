#include "motis/loader/parsers/hrd/service/rules_graph.h"

#include <string>

#include "motis/core/common/logging.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace motis::logging;

std::map<rules_graph::node_id, rules_graph::node> build_graph(
    std::vector<through_service_rule> const& ts_rules,
    std::vector<merge_split_rule> const& ms_rules) {
  scoped_timer timer("build service rules graph");

  std::map<rules_graph::node_id, rules_graph::node> nodes;
  for (auto const& rule : ts_rules) {
    auto& node_from = get_or_create(nodes, rule.service_key_from,
                                    []() { return rules_graph::node(); });
    auto& node_to = get_or_create(nodes, rule.service_key_to,
                                  []() { return rules_graph::node(); });
    node_from.ts_rules_out.push_back(
        rules_graph::edge(node_to, rule.eva_num, rules_graph::EVA_NUM_NOT_SET,
                          rule.bitfield_num));
    node_to.ts_rules_in.push_back(
        rules_graph::edge(node_from, rule.eva_num, rules_graph::EVA_NUM_NOT_SET,
                          rule.bitfield_num));
  }
  for (auto const& rule : ms_rules) {
    auto& node_from = get_or_create(nodes, rule.service_key_1,
                                    []() { return rules_graph::node(); });
    auto& node_to = get_or_create(nodes, rule.service_key_2,
                                  []() { return rules_graph::node(); });
    node_from.ms_rules.push_back(rules_graph::edge(
        node_to, rule.eva_num_begin, rule.eva_num_end, rule.bitfield_num));
    node_to.ms_rules.push_back(rules_graph::edge(
        node_from, rule.eva_num_begin, rule.eva_num_end, rule.bitfield_num));
  }
  return nodes;
}

rules_graph::rules_graph(std::vector<through_service_rule> const& ts_rules,
                         std::vector<merge_split_rule> const& ms_rules)
    : nodes_(std::move(build_graph(ts_rules, ms_rules))) {}

/*TODO (Tobias Raffel) remove after analysis */
std::string int_to_raw(uint64_t key) {
  return std::string(reinterpret_cast<char const*>(&key), sizeof(uint64_t));
}

/*TODO (Tobias Raffel) remove after analysis */
void rules_graph::print_graph() {
  for (auto const& entry : nodes_) {
    printf("[%d %s]: deg(THR_in)=%lu, deg(THR_out)=%lu, deg(MSR)=%lu\n",
           entry.first.first, int_to_raw(entry.first.second).c_str(),
           entry.second.ts_rules_in.size(), entry.second.ts_rules_out.size(),
           entry.second.ms_rules.size());
  }
  printf("num_nodes=%lu\n", nodes_.size());
}

}  // hrd
}  // loader
}  // motis
