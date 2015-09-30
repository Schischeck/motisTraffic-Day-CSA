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

struct ts_rule_resolvent {
  ts_rule_resolvent(through_service_rule const&, hrd_service const&);
  bool is_ending_service() const;

  through_service_rule const& rule_;
  hrd_service const& participant_;
};

struct ms_rule_resolvent {
  ms_rule_resolvent(merge_split_service_rule const&, hrd_service const&);

  merge_split_service_rule& rule_;
  hrd_service& participant_;
};

struct rules_graph {
  rules_graph(std::vector<through_service_rule>,
              std::vector<merge_split_service_rule>, bitfield_translator&);
  bool add(hrd_service const&);

  std::map<std::pair<int, uint64_t>, std::vector<ts_rule_resolvent>>
      ts_resolvents_;
  std::map<std::pair<int, uint64_t>, std::vector<ms_rule_resolvent>>
      mss_resolvents_;
  std::vector<through_service_rule> ts_rules_;
  std::vector<merge_split_service_rule> mss_rules_;
  std::vector<hrd_service> services_;
  bitfield_translator& bt_;
};

}  // hrd
}  // loader
}  // motis
