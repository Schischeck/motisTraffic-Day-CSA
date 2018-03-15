#include "motis/loader/hrd/parser/categories_parser.h"

#include "parser/arg_parser.h"
#include "parser/cstr.h"

#include "motis/core/common/logging.h"
#include "motis/loader/parser_error.h"
#include "motis/loader/util.h"

using namespace parser;
using namespace motis::logging;

namespace motis {
namespace loader {
namespace hrd {

std::map<uint32_t, category> parse_categories(loaded_file const& file,
                                              config const& c) {
  scoped_timer timer("parsing categories");
  bool ignore = false;
  std::map<uint32_t, category> categories;
  for_each_line_numbered(file.content(), [&](cstr line, int line_number) {
    if (ignore || line.len <= 1 || line.str[0] == '#' || line.str[0] == '%') {
      return;
    } else if (line.starts_with("<Produktwahltexte>")) {
      ignore = true;
      return;
    } else if (line.len < 20) {
      throw parser_error(file.name(), line_number);
    }

    auto const code = raw_to_int<uint32_t>(c.parse_field(line, c.cat_.code_));
    auto const output_rule =
        parse<uint8_t>(c.parse_field(line, c.cat_.output_rule_));
    auto const name = c.parse_field(line, c.cat_.name_).trim();
    categories[code] = {std::string(name.c_str(), name.length()), output_rule};
  });
  return categories;
}

}  // namespace hrd
}  // namespace loader
}  // namespace motis
