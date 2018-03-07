#include "motis/loader/hrd/parser/timezones_parser.h"

#include "boost/date_time/gregorian/gregorian_types.hpp"

#include "parser/arg_parser.h"

#include "motis/core/common/date_time_util.h"
#include "motis/loader/hrd/parser/basic_info_parser.h"
#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace boost::gregorian;
using namespace parser;

int eva_number(cstr str) { return parse<int>(str); }

int distance_to_midnight(cstr hhmm) { return hhmm_to_min(parse<int>(hhmm)); }

int bitfield_idx(cstr ddmmyyyy, date const& first_schedule_date) {
  date season_begin_date(parse<int>(ddmmyyyy.substr(4, size(4))),
                         parse<int>(ddmmyyyy.substr(2, size(2))),
                         parse<int>(ddmmyyyy.substr(0, size(2))));
  return (season_begin_date - first_schedule_date).days();
}

template <typename T>
timezones parse_timezones(loaded_file const& timezones_file,
                          loaded_file const& basic_data_file, T const& config) {
  auto const first_schedule_date = get_first_schedule_date(basic_data_file);

  timezones tz;
  for_each_line(timezones_file.content(), [&](cstr line) {
    if (line.length() == 15) {
      auto first_valid_eva_number =
          eva_number(parse_field(line, config.timezones.type1_first_valid_eva));
      auto it = tz.eva_to_tze_.find(first_valid_eva_number);
      verify(it != end(tz.eva_to_tze_),
             "missing timezone information for eva number: %d",
             first_valid_eva_number);

      tz.eva_to_tze_[eva_number(
          parse_field(line, config.timezones.type1_eva))] = it->second;
      return;
    }

    if (isdigit(line[0]) && line.length() >= 47) {
      boost::optional<season_entry> opt_season_entry;
      if (!line.substr(14, size(33)).trim().empty()) {
        opt_season_entry.emplace(
            distance_to_midnight(
                parse_field(line, config.timezones.type3_dst_to_midnight1)),
            bitfield_idx(
                parse_field(line, config.timezones.type3_bitfield_idx1),
                first_schedule_date),
            bitfield_idx(
                parse_field(line, config.timezones.type3_bitfield_idx2),
                first_schedule_date),
            distance_to_midnight(
                parse_field(line, config.timezones.type3_dst_to_midnight2)),
            distance_to_midnight(
                parse_field(line, config.timezones.type3_dst_to_midnight3)));
      }

      tz.timezone_entries_.push_back(std::make_unique<timezone_entry>(
          distance_to_midnight(
              parse_field(line, config.timezones.type2_dst_to_midnight)),
          opt_season_entry));

      tz.eva_to_tze_[eva_number(
          parse_field(line, config.timezones.type2_eva))] =
          tz.timezone_entries_.back().get();
    }
  });

  return tz;
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
