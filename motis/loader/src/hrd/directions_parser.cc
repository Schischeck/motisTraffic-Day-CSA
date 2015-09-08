#include "motis/loader/parsers/hrd/directions_parser.h"

#include "motis/loader/util.h"

namespace motis {
namespace loader {
namespace hrd {

std::map<uint64_t, std::string> parse_directions(
    loaded_file const& directions_file) {
  std::map<uint64_t, std::string> directions;
  for_each_line_numbered(directions_file.content,
                         [&directions](cstr line, int line_number) {
                           // TODO (Tobias Raffel) implement
                         });
  return directions;
}

}  // hrd
}  // loader
}  // motis
