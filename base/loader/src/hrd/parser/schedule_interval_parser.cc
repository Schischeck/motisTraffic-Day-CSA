#include "motis/loader/hrd/parser/schedule_interval_parser.h"

#include "parser/arg_parser.h"
#include "parser/util.h"

#include "motis/core/common/date_time_util.h"
#include "motis/core/common/logging.h"

using namespace flatbuffers;
using namespace parser;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

void verify_line_format(cstr s) {
  verify(
      s.len == 10 && (std::isdigit(s[0]) && std::isdigit(s[1]) && s[2] == '.' &&
                      std::isdigit(s[3]) && std::isdigit(s[4]) && s[5] == '.' &&
                      std::isdigit(s[6]) && std::isdigit(s[7]) &&
                      std::isdigit(s[8]) && std::isdigit(s[9])),
      "interval boundary [%.*s] invalid", static_cast<int>(s.len), s.str);
}

std::tuple<int, int, int> yyyymmdd(cstr ddmmyyyy) {
  return std::make_tuple(parse<int>(ddmmyyyy.substr(6, size(4))),
                         parse<int>(ddmmyyyy.substr(3, size(2))),
                         parse<int>(ddmmyyyy.substr(0, size(2))));
}

time_t str_to_unixtime(cstr s) {
  int year, month, day;
  std::tie(year, month, day) = yyyymmdd(s);
  return to_unix_time(year, month, day);
}

std::pair<cstr, cstr> mask_dates(cstr str) {
  auto from_line = parser::get_line(str);
  str += from_line.len + 1;
  auto to_line = parser::get_line(str);

  verify_line_format(from_line);
  verify_line_format(to_line);

  return std::make_pair(from_line, to_line);
}

Interval parse_interval(loaded_file const& basic_info_file) {
  scoped_timer timer("parsing schedule interval");
  cstr first_date;
  cstr last_date;
  std::tie(first_date, last_date) = mask_dates(basic_info_file.content());
  return Interval(str_to_unixtime(first_date), str_to_unixtime(last_date));
}

boost::gregorian::date get_first_schedule_date(loaded_file const& lf) {
  int year, month, day;
  std::tie(year, month, day) = yyyymmdd(mask_dates(lf.content()).first);
  return boost::gregorian::date(year, month, day);
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
