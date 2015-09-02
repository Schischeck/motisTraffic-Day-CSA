#include "motis/loader/parsers/hrd/schedule_interval_parser.h"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "parser/util.h"
#include "parser/arg_parser.h"

using namespace flatbuffers;
using namespace parser;

namespace motis {
namespace loader {

void verify_line_format(cstr s) {
  verify(
      s.len == 10 && (std::isdigit(s[0]) && std::isdigit(s[1]) && s[2] == '.' &&
                      std::isdigit(s[3]) && std::isdigit(s[4]) && s[5] == '.' &&
                      std::isdigit(s[6]) && std::isdigit(s[7]) &&
                      std::isdigit(s[8]) && std::isdigit(s[9])),
      "interval boundary [%.*s] invalid", static_cast<int>(s.len), s.str);
}

time_t str_to_unixtime(cstr s) {
  auto day = parse<int>(s.substr(0, size(2)));
  auto month = parse<int>(s.substr(3, size(2)));
  auto year = parse<int>(s.substr(6, size(4)));
  boost::posix_time::ptime t(boost::gregorian::date(year, month, day));
  boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
  return (t - epoch).total_seconds();
}

Interval parse_interval(loaded_file const& basic_info_file) {
  auto content = basic_info_file.content;

  auto from_line = parser::get_line(content);
  content += from_line.len + 1;
  auto to_line = parser::get_line(content);

  verify_line_format(from_line);
  verify_line_format(to_line);

  return Interval(str_to_unixtime(from_line), str_to_unixtime(to_line));
}

}  // loader
}  // motis
