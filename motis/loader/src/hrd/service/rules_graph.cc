#include "motis/loader/parsers/hrd/service/rules_graph.h"

#include <string>

#include "motis/core/common/logging.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace motis::logging;

rules_graph::rules_graph(std::vector<through_service_rule> ts_rules,
                         std::vector<merge_split_service_rule> mss_rules,
                         bitfield_translator& bt)
    : ts_rules_(std::move(ts_rules)),
      mss_rules_(std::move(mss_rules)),
      bt_(bt) {}

bool rules_graph::add(hrd_service const& s) {

  auto keys = collect_keys(s);
  for (auto const& key : keys) {
    auto ts_rules_it = std::lower_bound(
        std::begin(ts_rules_), std::end(ts_rules_),
        key);  // nur die service_1 keys sind aufeinanderfolgend :(((
  }
  auto mss_rules_it =
      std::lower_bound(std::begin(mss_rules_), std::end(mss_rules_), keys);

  return false;
}

ts_rule_resolvent::ts_rule_resolvent(through_service_rule const& rule,
                                     hrd_service const& participant)
    : rule_(rule), participant_(participant) {}

bool ts_rule_resolvent::is_ending_service() const {
  auto const& last_section = participant_.sections_.back();
  return std::make_pair(last_section.train_num,
                        raw_to_int<uint64_t>(last_section.admin)) ==
         rule_.service_key_1;
}

ms_rule_resolvent::ms_rule_resolvent(merge_split_service_rule const& rule,
                                     hrd_service const& participant)
    : rule_(rule), participant_(participant) {}

/*TODO (Tobias Raffel) remove after analysis */
std::string int_to_raw(uint64_t key) {
  return std::string(reinterpret_cast<char const*>(&key), sizeof(uint64_t));
}

}  // hrd
}  // loader
}  // motis
