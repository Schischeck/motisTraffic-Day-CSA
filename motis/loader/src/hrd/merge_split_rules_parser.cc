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

std::vector<merge_split_rule> parse_merge_split_rules(loaded_file const& src) {
  scoped_timer timer("parsing merge split rules");

  std::vector<merge_split_rule> rules;
  for_each_line(src.content, [&](cstr line) {
    if (line.len < 50) {
      return;
    }
    rules.push_back(
        {std::make_pair(parse<int>(line.substr(16, size(5))),
                        raw_to_int<uint64_t>(line.substr(23, size(6)))),
         std::make_pair(parse<int>(line.substr(30, size(5))),
                        raw_to_int<uint64_t>(line.substr(37, size(6)))),
         parse<int>(line.substr(0, size(7))),
         parse<int>(line.substr(8, size(7))),
         parse<int>(line.substr(44, size(6)))});
  });

  return rules;
}

}  // hrd
}  // loader
}  // motis
