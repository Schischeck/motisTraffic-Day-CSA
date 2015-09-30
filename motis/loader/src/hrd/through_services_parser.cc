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

bool through_service_rule::operator==(through_service_rule const& lhs,
                                      through_service_rule const& rhs) const {
  return lhs.service_key_1 == rhs.service_key_1 &&
         lhs.service_key_2 == rhs.service_key_2 && lhs.eva_num == rhs.eva_num &&
         lhs.mask == rhs.mask;
}

bool through_service_rule::operator<(through_service_rule const& lhs,
                                     through_service_rule const& rhs) const {
  return lhs.service_key_1 < rhs.service_key_1;
}

std::map<through_service_rule::key,
         std::vector<std::shared_ptr<through_service_rule>>>
parse_through_service_rules(loaded_file const& src, bitfield_translator bt) {
  scoped_timer timer("parsing through trains");

  std::map<through_service_rule::key,
           std::vector<std::shared_ptr<through_service_rule>>> rules;
  for_each_line_numbered(src.content, [&](cstr line, int line_number) {
    if (line.len < 40) {
      return;
    }

    auto it = bt.hrd_bitfields_.find(parse<int>(line.substr(34, size(6))));
    verify(it != std::end(bt.hrd_bitfields_), "missing bitfield: %s:%d",
           src.name, line_number);

    auto key_1 = std::make_pair(parse<int>(line.substr(0, size(5))),
                                raw_to_int<uint64_t>(line.substr(6, size(6))));
    auto key_2 = std::make_pair(parse<int>(line.substr(21, size(5))),
                                raw_to_int<uint64_t>(line.substr(27, size(6))));
    auto rule =
        std::make_shared({key_1, key_2, parse<int>(line.substr(13, size(7))),
                          std::move(it->second)});
    rules[key_1].push_back(rule);
    rules[key_2].push_back(rule);
  });

  std::sort(std::begin(rules), std::end(rules));

  return rules;
}

}  // hrd
}  // loader
}  // motis
