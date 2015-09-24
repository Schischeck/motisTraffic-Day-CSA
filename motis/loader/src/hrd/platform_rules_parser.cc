#include "motis/loader/parsers/hrd/platform_rules_parser.h"

#include "parser/cstr.h"
#include "parser/arg_parser.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/parsers/hrd/bitfields_parser.h"

using namespace parser;
using namespace flatbuffers;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

platform_rules parse_platform_rules(loaded_file file,
                                    flatbuffers::FlatBufferBuilder& b) {
  scoped_timer timer("parsing platform rules");
  platform_rules prs;
  std::map<uint64_t, Offset<String>> platform_names;

  for_each_line_numbered(file.content, [&](cstr line, int line_number) {
    if (line.len == 0) {
      return;
    } else if (line.len != 41) {
      throw parser_error(file.name, line_number);
    }

    auto eva_num = parse<int>(line.substr(0, size(7)));
    auto train_num = parse<int>(line.substr(8, size(5)));
    auto train_admin = raw_to_int<uint64_t>(line.substr(14, size(6)));
    auto platform_name_str = line.substr(21, size(8)).trim();
    auto platform_name = raw_to_int<uint64_t>(platform_name_str);
    auto time =
        hhmm_to_min(parse<int>(line.substr(30, size(4)).trim(), TIME_NOT_SET));
    auto bitfield = parse<int>(line.substr(35, size(6)).trim(), ALL_DAYS_KEY);

    // Resolve platform name (create it if not found)
    auto platform_name_it = platform_names.find(platform_name);
    if (platform_name_it == end(platform_names)) {
      std::tie(platform_name_it, std::ignore) = platform_names.insert(
          std::make_pair(platform_name, to_fbs_string(b, platform_name_str)));
    }

    prs[std::make_tuple(eva_num, train_num, train_admin)].push_back(
        {platform_name_it->second, bitfield, time});
  });
  return prs;
}

}  // hrd
}  // loader
}  // motis
