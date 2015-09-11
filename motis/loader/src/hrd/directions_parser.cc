#include "motis/loader/parsers/hrd/directions_parser.h"

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"

namespace motis {
namespace loader {
namespace hrd {

std::map<uint64_t, std::string> parse_directions(
    loaded_file const& directions_file) {
  std::map<uint64_t, std::string> directions;
  for_each_line_numbered(
      directions_file.content, [&](cstr line, int line_number) {
        if (line.length() < 9 && line[7] == ' ') {
          throw parser_error(directions_file.name, line_number);
        } else {
          auto const text = line.substr(8);
          directions[raw_to_int<uint64_t>(line.substr(1, size(7)))] =
              std::string(text.str, text.len);
        }
      });
  return directions;
}

}  // hrd
}  // loader
}  // motis
