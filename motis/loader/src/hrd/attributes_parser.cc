#include "motis/loader/parsers/hrd/attributes_parser.h"

#include "motis/loader/util.h"

#include "motis/loader/parser_error.h"

using namespace parser;
using namespace flatbuffers;

namespace motis {
namespace loader {
namespace hrd {

std::vector<Offset<Attribute>> parse_attributes(loaded_file file,
                                                FlatBufferBuilder& b) {
  std::vector<Offset<Attribute>> attributes;
  for_each_line_numbered(file.content, [&](cstr line, int line_number) {
    if (line.len == 0 || line.str[0] == '#') {
      return;
    } else if (line.len < 13) {
      throw parser_error(file.name, line_number);
    }
    auto code = to_fbs_string(b, line.substr(0, size(2)));
    auto text = to_fbs_string(b, line.substr(12, line.len - 1));
    attributes.emplace_back(CreateAttribute(b, code, text));
  });
  return attributes;
}

}  // hrd
}  // loader
}  // motis
