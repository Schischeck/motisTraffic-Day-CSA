#include "motis/loader/hrd/parser/timezones_parser.h"

#include "boost/date_time/gregorian/gregorian_types.hpp"

#include "parser/arg_parser.h"

#include "motis/loader/util.h"
#include "motis/loader/hrd/parser/schedule_interval_parser.h"

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

timezones parse_timezones(loaded_file const& timezones_file,
                          loaded_file const& basic_data_file) {
  auto const first_schedule_date = get_first_schedule_date(basic_data_file);

  timezones tz;
  for_each_line(timezones_file.content(), [&](cstr line) {
    if (line.length() == 15) {
      auto first_valid_eva_number = eva_number(line.substr(8, size(7)));
      auto it = tz.entries_.find(first_valid_eva_number);
      verify(it != end(tz.entries_),
             "missing timezone information for eva number: %d",
             first_valid_eva_number);

      tz.entries_[eva_number(line.substr(0, size(7)))] = it->second;
      return;
    }
    if (isdigit(line[0]) && line.length() >= 47) {

      ranges::optional<season_entry> opt_season_entry;
      if (!line.substr(14, size(33)).trim().empty()) {
        opt_season_entry = {
            distance_to_midnight(line.substr(14, size(5))),
            bitfield_idx(line.substr(20, size(8)), first_schedule_date),
            distance_to_midnight(line.substr(29, size(4))),
            bitfield_idx(line.substr(34, size(8)), first_schedule_date),
            distance_to_midnight(line.substr(43, size(4)))};
      }

      tz.timezone_entries_.push_back(make_unique<timezone_entry>(
          distance_to_midnight(line.substr(8, size(5))), opt_season_entry));

      tz.entries_[eva_number(line.substr(0, size(7)))] =
          tz.timezone_entries_.back().get();
    }
  });

  return tz;
}

}  // hrd
}  // loader
}  // motis
