#include "motis/loader/parsers/hrd/categories_parser.h"

#include "parser/cstr.h"
#include "parser/arg_parser.h"

#include "motis/loader/util.h"
#include "motis/loader/parser_error.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace parser;

std::map<uint32_t, category> parse_categories(
    loaded_file const& categories_file) {

  std::map<uint32_t, category> categories;
  for_each_line_numbered(
      categories_file.content, [&](cstr line, int line_number) {
        if (line.len == 0 || line.str[0] == '#') {
          return;
        } else if (line.len < 20) {
          throw parser_error(categories_file.name, line_number);
        }

        auto const code = raw_to_int<uint32_t>(line.substr(0, size(3)));
        auto const output_rule = parse<int>(line.substr(9, size(2)));
        auto const name = line.substr(12, size(8)).trim();
        categories[code] = {std::string(name.c_str(), name.length()),
                            static_cast<CategoryOutputRule>(output_rule)};
      });

  return categories;
}

}  // hrd
}  // loader
}  // motis
