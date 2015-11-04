#include "motis/loader/parsers/hrd/attributes_parser.h"

#include "parser/cstr.h"

#include "motis/core/common/logging.h"
#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"

using namespace parser;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

std::map<uint16_t, std::string> parse_attributes(loaded_file const& file) {
  scoped_timer timer("parsing attributes");
  std::map<uint16_t, std::string> attributes;
  for_each_line_numbered(file.content(), [&](cstr line, int line_number) {
    if (line.len == 0 || line.str[0] == '#') {
      return;
    } else if (line.len < 13) {
      throw parser_error(file.name(), line_number);
    }
    auto code = line.substr(0, size(2));
    auto text = line.substr(12, line.len - 1);
    attributes[raw_to_int<uint16_t>(code)] = std::string(text.str, text.len);
  });
  return attributes;
}

}  // hrd
}  // loader
}  // motis
