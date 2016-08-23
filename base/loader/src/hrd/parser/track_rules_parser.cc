#include "motis/loader/hrd/parser/track_rules_parser.h"

#include "parser/arg_parser.h"
#include "parser/cstr.h"

#include "motis/core/common/date_time_util.h"
#include "motis/core/common/logging.h"
#include "motis/loader/hrd/parser/bitfields_parser.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/util.h"

using namespace parser;
using namespace flatbuffers64;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

track_rules parse_track_rules(loaded_file const& file,
                                    flatbuffers64::FlatBufferBuilder& b) {
  scoped_timer timer("parsing track rules");
  track_rules prs;
  std::map<uint64_t, Offset<String>> track_names;

  for_each_line_numbered(file.content(), [&](cstr line, int line_number) {
    if (line.len == 0) {
      return;
    } else if (line.len != 41) {
      throw parser_error(file.name(), line_number);
    }

    auto eva_num = parse<int>(line.substr(0, size(7)));
    auto train_num = parse<int>(line.substr(8, size(5)));
    auto train_admin = raw_to_int<uint64_t>(line.substr(14, size(6)));
    auto track_name_str = line.substr(21, size(8)).trim();
    auto track_name = raw_to_int<uint64_t>(track_name_str);
    auto time =
        hhmm_to_min(parse<int>(line.substr(30, size(4)).trim(), TIME_NOT_SET));
    auto bitfield = parse<int>(line.substr(35, size(6)).trim(), ALL_DAYS_KEY);

    // Resolve track name (create it if not found)
    auto track_name_it = track_names.find(track_name);
    if (track_name_it == end(track_names)) {
      std::tie(track_name_it, std::ignore) = track_names.insert(
          std::make_pair(track_name,
                         to_fbs_string(b, track_name_str, "ISO-8859-1")));
    }

    prs[std::make_tuple(eva_num, train_num, train_admin)].push_back(
        {track_name_it->second, bitfield, time});
  });
  return prs;
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
