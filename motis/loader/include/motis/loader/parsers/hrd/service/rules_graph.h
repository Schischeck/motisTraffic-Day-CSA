#pragma once

#include <cinttypes>
#include <vector>
#include <map>

#include "parser/cstr.h"

#include "motis/loader/util.h"
#include "motis/loader/parsers/hrd/through_services_parser.h"
#include "motis/loader/parsers/hrd/merge_split_rules_parser.h"
#include "motis/loader/parsers/hrd/service/hrd_service.h"

namespace motis {
namespace loader {
namespace hrd {

struct rules_graph {
  static constexpr int EVA_NUM_NOT_SET = -1;
  struct edge;
  struct node {
    std::vector<hrd_service> involved_services;
    std::vector<edge> ts_rules_in, ts_rules_out, ms_rules;
  };
  struct edge {
    edge(node& target, int eva_num_1, int eva_num_2, int bitfield_num)
        : target(target),
          eva_num_1(eva_num_1),
          eva_num_2(eva_num_2),
          bitfield_num(bitfield_num){};
    node& target;
    int const eva_num_1, eva_num_2, bitfield_num;
  };

  rules_graph(std::vector<through_service_rule> const&,
              std::vector<merge_split_rule> const&);

  void print_graph();

  // node_id := (service number, administration)
  typedef std::pair<int, uint64_t> node_id;
  std::map<node_id, node> nodes_;
};

}  // hrd
}  // loader
}  // motis
