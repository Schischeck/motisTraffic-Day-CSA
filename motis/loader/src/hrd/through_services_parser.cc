#include "motis/loader/parsers/hrd/through_services_parser.h"

#include "parser/cstr.h"
#include "parser/arg_parser.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"

using namespace parser;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

std::vector<through_service_rule> parse_through_service_rules(
    loaded_file const& src) {
  scoped_timer timer("parsing through trains");

  std::vector<through_service_rule> rules;
  for_each_line(src.content, [&](cstr line) {
    if (line.len < 40) {
      return;
    }
    rules.push_back(
        {std::make_pair(parse<int>(line.substr(0, size(5))),
                        raw_to_int<uint64_t>(line.substr(6, size(6)))),
         std::make_pair(parse<int>(line.substr(21, size(5))),
                        raw_to_int<uint64_t>(line.substr(27, size(6)))),
         parse<int>(line.substr(13, size(7))),
         parse<int>(line.substr(34, size(6)))});
  });

  return rules;
}

}  // hrd
}  // loader
}  // motis
