#include "motis/loader/parsers/hrd/directions_parser.h"

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"

namespace motis {
namespace loader {
namespace hrd {

std::map<uint64_t, std::string> parse_directions(loaded_file const& file) {
  std::map<uint64_t, std::string> directions;
  for_each_line_numbered(file.content_, [&](cstr line, int line_number) {
    if (line.length() < 9 && line[7] == ' ') {
      throw parser_error(file.name_, line_number);
    } else {
      auto const text = line.substr(8);
      directions[raw_to_int<uint64_t>(line.substr(0, size(7)))] =
          std::string(text.str, text.len);
    }
  });
  return directions;
}

}  // hrd
}  // loader
}  // motis
