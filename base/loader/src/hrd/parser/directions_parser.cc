#include "motis/loader/hrd/parser/directions_parser.h"

#include "motis/loader/parser_error.h"
#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

template <typename T>
std::map<uint64_t, std::string> parse_directions(loaded_file const& file,
                                                 T const& config) {
  std::map<uint64_t, std::string> directions;
  for_each_line_numbered(file.content(), [&](cstr line, int line_number) {
    if (line.length() < 9 && line[7] == ' ') {
      throw parser_error(file.name(), line_number);
    } else {
      auto const text = parse_field(line, config.directions.text);
      directions[raw_to_int<uint64_t>(parse_field(
          line, config.directions.eva))] = std::string(text.str, text.len);
    }
  });
  return directions;
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
