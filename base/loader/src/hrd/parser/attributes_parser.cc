#include "motis/loader/hrd/parser/attributes_parser.h"

#include "parser/cstr.h"

#include "motis/core/common/logging.h"
#include "motis/loader/hrd/parse_config.h"
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

template <typename T>
std::map<uint16_t, std::string> parse_attributes(loaded_file const& file,
                                                 T const& config) {
  scoped_timer timer("parsing attributes");
  std::map<uint16_t, std::string> attributes;
  for_each_line_numbered(file.content(), [&](cstr line, int line_number) {
    if (line.len == 0 || line.str[0] == '#') {
      return;
    } else if (line.len < 13 || (is_multiple_spaces(line) && line.len < 22)) {
      throw parser_error(file.name(), line_number);
    }
    auto code = parse_field(line, config.attribute.code);
    auto text = is_multiple_spaces(line)
                    ? parse_field(line, config.attribute.text_mul_spaces)
                    : parse_field(line, config.attribute.text_normal);
    attributes[raw_to_int<uint16_t>(code)] = std::string(text.str, text.len);
  });
  return attributes;
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
