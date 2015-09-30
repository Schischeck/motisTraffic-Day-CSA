#include "motis/loader/parsers/hrd/merge_split_rules_parser.h"

#include "parser/cstr.h"
#include "parser/arg_parser.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"

using namespace parser;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

bool merge_split_service_rule::operator==(
    merge_split_service_rule const& lhs,
    merge_split_service_rule const& rhs) const {
  return lhs.service_key_1 == rhs.service_key_1 &&
         lhs.service_key_2 == rhs.service_key_2 &&
         lhs.eva_num_begin == rhs.eva_num_begin &&
         lhs.eva_num_end == rhs.eva_num_end && lhs.mask == rhs.mask;
}

std::vector<merge_split_service_rule> parse_merge_split_service_rules(
    loaded_file const& src, bitfield_translator bt) {
  scoped_timer timer("parsing merge split rules");

  std::vector<merge_split_service_rule> rules;
  for_each_line_numbered(src.content, [&](cstr line, int line_number) {
    if (line.len < 53) {
      return;
    }

    auto it = bt.hrd_bitfields_.find(parse<int>(line.substr(47, size(6))));
    verify(it != std::end(bt.hrd_bitfields_), "missing bitfield: %s:%d",
           src.name, line_number);

    rules.push_back(
        {std::make_pair(parse<int>(line.substr(18, size(5))),
                        raw_to_int<uint64_t>(line.substr(25, size(6)).trim())),
         std::make_pair(parse<int>(line.substr(33, size(5))),
                        raw_to_int<uint64_t>(line.substr(40, size(6)).trim())),
         parse<int>(line.substr(0, size(7))),
         parse<int>(line.substr(9, size(7))), std::move(it->second)});
  });

  return rules;
}

}  // hrd
}  // loader
}  // motis
