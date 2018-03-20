#include "motis/loader/hrd/parser/attributes_parser.h"

#include "parser/cstr.h"

#include "motis/core/common/logging.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/util.h"

using namespace parser;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

bool is_multiple_spaces(cstr line) {
  return line.substr(2, size(3)).trim().empty();
}

std::map<uint16_t, std::string> parse_attributes(loaded_file const& file,
                                                 config const& c) {
  scoped_timer timer("parsing attributes");
  std::map<uint16_t, std::string> attributes;
  for_each_line_numbered(file.content(), [&](cstr line, int line_number) {
    if (line.len == 0 || line.str[0] == '#') {
      return;
    } else if (line.len < 13 || (is_multiple_spaces(line) && line.len < 22)) {
      throw parser_error(file.name(), line_number);
    }
    auto code = c.parse_field(line, c.att_.code_);
    auto text = is_multiple_spaces(line)
                    ? c.parse_field(line, c.att_.text_mul_spaces_)
                    : c.parse_field(line, c.att_.text_normal_);
    attributes[raw_to_int<uint16_t>(code)] =
        std::string(text.str, text.len - 1);
  });
  return attributes;
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
