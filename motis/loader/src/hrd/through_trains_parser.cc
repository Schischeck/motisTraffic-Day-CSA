#include "motis/loader/parsers/hrd/through_trains_parser.h"

#include "parser/cstr.h"
#include "parser/arg_parser.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"

using namespace parser;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

through_trains_map parse_through_train_rules(loaded_file const& src) {
  scoped_timer timer("parsing through trains");

  through_trains_map rules;
  for_each_line(src.content, [&](cstr line) {
    if (line.len < 41) {
      return;
    }

    auto train_no_1 = parse<int>(line.substr(0, size(5)));
    auto train_no_2 = parse<int>(line.substr(21, size(5)));
    auto admin_1 = raw_to_int<uint64_t>(line.substr(6, size(6)));
    auto admin_2 = raw_to_int<uint64_t>(line.substr(27, size(6)));
    auto eva_num = parse<int>(line.substr(13, size(7)));
    auto traffic_days = parse<int>(line.substr(34, size(6)));

    auto r = std::make_shared<through_train_rule>(
        traffic_days, boost::optional<hrd_service>(),
        boost::optional<hrd_service>());

    rules[std::make_tuple(true, train_no_1, admin_1, eva_num)].emplace_back(r);
    rules[std::make_tuple(false, train_no_2, admin_2, eva_num)].emplace_back(r);
  });

  return rules;
}

}  // hrd
}  // loader
}  // motis
